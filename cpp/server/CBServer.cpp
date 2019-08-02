#include "CBServer.hpp"

#include <unistd.h>
#include <iostream>
#include <dirent.h>
#include <pwd.h>
#include <fstream>

#include <utility/Directory.h>
#include <imaging/Imaging.h>
#include <utility/JSONHelper.h>
#include <utility/Diagnostics.h>
#include <utility/CommonUtility.h>
#include <cbcommon/CB_Config.h>

#include <cbar/CBAR_CallbackI.hpp>
#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/scene/scene.h>
#include <cbar/pipeline/rendering/rendering.h>

#define LOG_JSON 0

using namespace cbpipe;
using namespace cbscene;

struct CBARServerCallback : CBAR_CallbackI {
    
    CBARServerCallback(CBServer *context) : CBAR_CallbackI((void *)context) {
        
    }
    
    virtual void historyChanged(std::shared_ptr<cbpipe::UndoState> undo, bool forward);
    
    virtual void colorsCallback(std::vector<cbar::ColorInfo>&colors);
    
    virtual void exposureCallback(cv::Point2f exposurePoint);
    
    virtual void focusCallback(cv::Point2f focusPoint);
};

struct CBARServerAssetCallback : CBAR_MaskedAssetCallbackI {
    
    CBARServerAssetCallback(CBServer *context) : CBAR_MaskedAssetCallbackI() {
        
    }
    
    virtual void attachedToScene();
    
    virtual void detachedFromScene();
    
    virtual void touchedAtPoint(cbpipe::TouchPoint touch);
    
    virtual void rotatedBy(cbpipe::TouchPoint touch);
};

struct CBServer::Impl
{
    Impl(CBServer *parent) : m_parent(parent) {
        m_callback = std::shared_ptr<CBAR_CallbackI>(new CBARServerCallback(parent));
        m_assetCallback = std::shared_ptr<CBARServerAssetCallback>(new CBARServerAssetCallback(parent));
        
        m_renderingPipeline = std::shared_ptr<CBP_RenderingEngine>(new cbpipe::CBP_RenderingEngine(parent, m_callback));
        m_renderingPipeline->initialize(m_renderingPipeline);
    }
    ~Impl() {
        
    }
    
    const char *getUserName() {
        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        if (pw)
        {
            return pw->pw_name;
        }
        
        return "";
    }
    
    std::string serializeJSON(Json::Value &rootNode, bool result, std::string projectData, std::string error="") {
        
        rootNode["api"] = API_VERSION;
        rootNode["result"] = result;

        if (error.length()) {
            rootNode["error"] = error;
        }
        
        std::string jsonOut = JSONHelper::jsonToString(rootNode);
        
        jsonOut.insert(jsonOut.length() - 1, projectData);
        
#if LOG_JSON
        std::string logOutputPath = _baseDirectory + "/json_output.js";
        
        std::ofstream outputFile;
        outputFile.open(logOutputPath.c_str());
        outputFile << jsonOut;
        outputFile.close();
#endif
        
        return jsonOut;
    }
    
    inline std::string getProjectUrl(std::string itemPath) {
        return string_sprintf("%s/%s", m_scene->getSceneID().c_str(), itemPath.c_str());
    }
    
    inline std::string getLocalProjectRoot(std::string sceneID) {
        std::string relPath = string_sprintf("projects/%s", sceneID.c_str());
        return getWorkingAssetPath(relPath);
    }
    
    inline std::string getLocalProjectPath(std::string itemPath) {
        return string_sprintf("%s/%s", getLocalProjectRoot(m_scene->getSceneID()).c_str(), itemPath.c_str());
    }
    
    bool generateSceneFromImage(const std::string &imagePath, bool masked) {
        
        if (!Directory::file_exists(imagePath.c_str())) {
            CBError("ERROR: Image at path %s not found.", imagePath.c_str());
            return false;
        }

        cv::Mat image = cv::imread(imagePath.c_str(), cv::IMREAD_COLOR);

        if (image.empty()) {
            CBError("ERROR: Image at path %s could not load", imagePath.c_str());
            return false;
        }
        
        m_needsScenePreparation = true;
        m_scene = make_shared<cbscene::CBAR_Scene>(genAssetFunction(), image, "", nullptr);
        
        return true;
    }
    
    bool loadScene(const std::string &sceneID, const Json::Value& inputJSON, bool masked) {
        
        std::string scenePath = getLocalProjectRoot(sceneID);
        
        if (!Directory::exists(scenePath.c_str())) {
            CBError("ERROR: Scene at path %s not found.", scenePath.c_str());
            return false;
        }

        CBLog("Loading scene at path %s", scenePath.c_str());
        m_scene = make_shared<cbscene::CBAR_Scene>(genAssetFunction(), sceneID, (void *)this);
        
        if (inputJSON.isMember("sceneData")) {
            const Json::Value& sceneData = inputJSON["sceneData"];
            m_scene->loadFromDirectory(scenePath, 0, &sceneData);
        } else {
            m_scene->loadFromDirectory(scenePath, 0);
        }
        
        CBLog("Successfully loaded scene at path %s", scenePath.c_str());
        //project.loadImage(image, masked);
        
        m_needsScenePreparation = true;
        
        return true;
    }
    
    void prepareScene() {
        m_renderingPipeline->setScene(m_scene);
        
        cv::Size imageSize = m_scene->getVideoFrame()->frameSize();
        
        m_renderingPipeline->prepareViewport(nullptr, nullptr, imageSize.width, imageSize.height,
                                             imageSize.width, imageSize.height, 1.1,
                                             0, Eigen::Matrix3f::Identity());
        
        //prepare viewport?

        m_needsScenePreparation = false;
    }
    
    //typedef std::function<std::shared_ptr<cbscene::CBAR_Asset> (CBAR_Asset::asset_type assetType, const std::string &assetID)> generate_asset_fn;
    
    std::vector<std::shared_ptr<cbscene::CBAR_Asset>> m_assets;
    
    cbscene::generate_asset_fn genAssetFunction() {
        
        return [this](cbscene::CBAR_Asset::asset_type assetType, const std::string &assetID) {
            
            std::shared_ptr<cbscene::CBAR_Asset> asset;
            
            if (!this) return asset;

            switch (assetType) {
                case cbscene::CBAR_Asset::asset_type_paint: {
                    asset = std::make_shared<cbscene::CBAR_Paint>(assetID, m_assetCallback);
                break;
                }
                case cbscene::CBAR_Asset::asset_type_floor: {
                    asset= std::make_shared<cbscene::CBAR_Floor>(assetID, m_assetCallback);
                    break;
                }
                case cbscene::CBAR_Asset::asset_type_model: {
                    asset = std::make_shared<cbscene::CBAR_Model>(assetID, m_assetCallback);
                    break;
                }
                default:
                    
                    break;
            }
            if (asset) {
                this->m_assets.push_back(asset);//retain
            }
            return asset;
        };
    }
    
    /*
     {   "version":"2.07",
     "inputImage":"living-room-1-masked.png",
     "project":"3BAC8DBC-6DE6-4D93-A724-D301AD65837C",
     "commands":[
     {"command":"setPaintColor","color":[200,200,0,0]},
     {"command":"fillAtPoint","point":[100,100]}
     ]
     } */
    std::string executeJSON(const std::string &json) {
        
#if LOG_JSON
        std::string logInputPath = _baseDirectory + "/json_input.js";
        
        std::ofstream inputFile;
        inputFile.open(logInputPath.c_str());
        inputFile << json;
        inputFile.close();
#endif

        Json::Value inputJSON;
        Json::Value outputJSON;
        std::string errors;
        
        if (!JSONHelper::parseJSONString(json, inputJSON, &errors)) {
            CBLog("Failed to parse json file: %s\n %s\n", errors.c_str(), json.c_str());
            return serializeJSON(outputJSON, false, "", errors);
        }
        
        std::cerr << "Running JSON: " << json << std::endl;
        //CBLog("Running JSON: %s", json.c_str());
        
        bool success = true;
        
        std::string sceneID = m_sceneID;
        if (inputJSON.isMember("sceneID") && !inputJSON["sceneID"].isNull() && inputJSON["sceneID"].asString().length() > 0) {
            sceneID = inputJSON["sceneID"].asString();
        }
        
        if (sceneID.size()) {
            success = loadScene(sceneID, inputJSON, false);
            
            CBError("Successfully loaded scene");
            if (success) {
                 prepareScene();
            }
            else {
                std::string scenePath = getLocalProjectRoot(sceneID);
                CBError("Could not find scene %s at path %s\n", sceneID.c_str(), scenePath.c_str());
                return serializeJSON(outputJSON, false, "", "Could not find scene " + sceneID);
            }
        }
        
        //run commands
        const Json::Value& commandValues = inputJSON["commands"];
        
        bool needsRefresh = false;

        Json::Value executedCommands;
        std::string errorMessage;
        
        int numCommands = commandValues.size();
        
        if (numCommands > 1) {
            CBLog("Executing %d commands.", numCommands);
        }
        
        for (int i = 0; success && i < numCommands; i++) {
            
            const Json::Value& command = commandValues[i];
            bool thisCommandNeedsRefresh = false;
            
            std::string commandError;
            //TODO: maybe throw?
            success = executeJSONCommand(command, thisCommandNeedsRefresh, commandError);
            
            if (success) {
                executedCommands.append(command["command"].asString());
            } else {
                errorMessage = string_sprintf("Command '%s' failed. %s", command["command"].asString().c_str(), commandError.c_str());
                break;
            }
            
            if (m_needsScenePreparation) {
                prepareScene();
            }
            
            if (thisCommandNeedsRefresh && m_scene->getAssets().size()) {
                if (auto asset = m_scene->getSelectedAsset()) {
                    //CBLog("Calling render for asset %s", asset->getAssetID().c_str());
                    asset->getRenderer()->handleFrame(m_scene->getVideoFrame());
                }
            }
            
            needsRefresh |= thisCommandNeedsRefresh;
        }

        if (success) {
            
            if (numCommands > 1) {
                CBLog("All %d commands were successful", numCommands);
            } else {
                CBLog("Commands was successful");
            }
            
            m_renderingPipeline->getAnalyzer()->analyzeFrame(m_scene->getVideoFrame());
        }
        else {
            CBError("ERROR: Command %s unsuccessful: %s", errorMessage.c_str(), json.c_str());
            return serializeJSON(outputJSON, false, "", errorMessage);
        };
        
        m_sceneID = m_scene->getSceneID();
        
        outputJSON["sceneID"] = m_scene->getSceneID();
        outputJSON["sceneImage"] = getProjectUrl("scene.png");
        outputJSON["previewImage"] = getProjectUrl("preview.jpg");
        outputJSON["brushSize"] = m_parent->getBrushRadius();
        outputJSON["toolMode"] = m_parent->getToolMode();
        outputJSON["needsRefresh"] = needsRefresh;
        outputJSON["executedCommands"] = executedCommands;
        
        std::string jsonString;
        if (success && m_scene) {
        
            auto savePath = getLocalProjectRoot(m_scene->getSceneID());
            std::string projectJSON = m_scene->saveToDirectory(savePath, false);
            
            jsonString = std::string(",\"sceneData\":") + projectJSON;
        }
        
        return serializeJSON(outputJSON, success, jsonString);
    }
    
    bool executeJSONCommand(const Json::Value& commandValue, bool &needsRefresh, std::string &errorMessage) {
        
        needsRefresh = false;
        std::string command = commandValue["command"].asString();
        
        CBLog("Executing command %s", JSONHelper::jsonToString(commandValue).c_str());
        
        if (command.compare("loadImage") == 0) {
            if (!commandValue.isMember("image") || !commandValue["image"].isString()) {
                errorMessage = "image json property missing";
                return false;
            }
            
            std::string imageName = commandValue["image"].asString();
            bool masked = false;
            if (commandValue.isMember("masked")) {
                masked = commandValue["masked"].asBool();
            }
            
            needsRefresh = true;
            //CBAR_Scene project;
            std::string imagePath = getUserAssetPath(imageName);
            return generateSceneFromImage(imagePath, masked);
        }
        else if (command.compare("syncData") == 0) {
            if (commandValue.isMember("screenshot")) {
                std::string previewPath = commandValue["screenshot"].asString();
            
                if (Directory::file_exists(previewPath.c_str())) {
                    std::string finalPath = getLocalProjectPath("preview.jpg");
                    CBLog("Moving screenshot file from %s to %s", previewPath.c_str(), finalPath.c_str());
                    rename( previewPath.c_str(), finalPath.c_str());
            return true;
                } else {
                    errorMessage = string_sprintf("File %s not found", previewPath.c_str());
                    return false;
                }
            }
            return true;//just synced json
        }
        else if (command.compare("appendAsset") == 0) {
            if (commandValue.isMember("type") && commandValue["type"].isInt()) {
                cbscene::CBAR_Asset::asset_type assetType = (cbscene::CBAR_Asset::asset_type) commandValue["type"].asInt();
                auto asset = genAssetFunction()(assetType, "");
                m_scene->appendAsset(asset);
                return true;
            } else {
                errorMessage = "type json property missing";
            }
        }
        else if (command.compare("undoState") == 0) {
            if (m_scene->getUndoSize()) {
                bool result = m_scene->undoState();
                needsRefresh = result;
                return result;
            }
            errorMessage = "Nothing to undo.";
            return false;
        }
        else if (command.compare("redoState") == 0) {
            if (m_scene->getRedoSize()) {
                bool result = m_scene->redoState();
                needsRefresh = result;
                return result;
            }
            errorMessage = "Nothing to redo.";
            return false;
        }
        else if (command.compare("clearAll") == 0) {
            bool result = true;
            m_scene->clearAssets();
            needsRefresh = true;
            return result;
        }
        else if (command.compare("setPaintColor") == 0) {
            auto paintAsset = std::dynamic_pointer_cast<CBAR_Paint>(m_scene->getSelectedAsset());
            if (paintAsset && commandValue.isMember("color") && commandValue["color"].isArray()) {
                cv::Scalar color = JSONHelper::getScalarJSONValue(commandValue["color"]);
                paintAsset->setColor(color);
                return true;
            } else {
                errorMessage = "color json property missing";
            }
        }
        else if (command.compare("setSelectedAssetID") == 0) {
            if (commandValue.isMember("assetID") && commandValue["assetID"].isString()) {
                std::string assetID = commandValue["assetID"].asString();
                m_scene->setSelectedAssetID(assetID);
                return true;
            } else {
                errorMessage = "assetID json property missing";
            }
            
        }
        else if (command.compare("fillAtPoint") == 0) {
            
            if (!commandValue.isMember("point") || !commandValue["point"].isArray()) {
                errorMessage = "point json property missing";
                return false;
            }
            
            if (!m_scene->getSelectedAsset()) {
                errorMessage = "No asset provided";
                return false;
            }
            
            cv::Point point = JSONHelper::getCVPointJSONValue(commandValue["point"]);
            
            cbpipe::TouchPoint touchPoint;
            touchPoint.toolMode = ToolModeFill;
            touchPoint.origin = point;
            
            touchPoint.step = TouchStepBegan;
            m_scene->getSelectedAsset()->touchedAtPoint(touchPoint);
            
            touchPoint.step = TouchStepEnded;
            m_scene->getSelectedAsset()->touchedAtPoint(touchPoint);
            
            needsRefresh = true;
            
            return true;
        }
        else if (command.compare("appendNewAsset") == 0) {
            //painter.appendNewLayer();
            return true;
        }
        else if (command.compare("setTransparency") == 0) {
            bool result = true;
            int transparency = commandValue["transparency"].asInt();
            //painter.editLayer()->setTransparency((Transparency) transparency);
            return result;
        }
        else if (command.compare("setSheen") == 0) {
            bool result = true;
            int sheen = commandValue["sheen"].asInt();
            //painter.editLayer()->setSheen((Sheen)sheen);
            return result;
        }
        else if (command.compare("setLighting") == 0) {
            bool result = true;
            int lighting = commandValue["lighting"].asInt();
            //painter.setSimulatedLighting((LightingType)lighting);
            return result;
        }
        else if (command.compare("setToolMode") == 0) {
            if (!commandValue.isMember("toolMode") || !commandValue["toolMode"].isInt()) {
                errorMessage = "toolMode json property missing";
                return false;
            }
            ToolMode toolMode = (ToolMode) commandValue["toolMode"].asInt();
            m_parent->setToolMode(toolMode);
            return true;
        }
        else if (command.compare("brushAtPoints") == 0 || command.compare("eraseAtPoints") == 0) {
            
            if (!commandValue.isMember("points") || !commandValue["points"].isArray()) {
                errorMessage = "points json property missing";
                return false;
            }
                
            const Json::Value& pointValues = commandValue["points"];
            bool success = true;
            
            bool brushTapEnabled = false; //painter.brushTapFillEnabled();
            if (pointValues.size() > 2) {
                //painter.setBrushTapFillEnabled(false);
            }
            
            ToolMode toolMode = ToolModePaintbrush;
            
            if (command.compare("brushAtPoints") == 0) {
                toolMode = ToolModePaintbrush;
            } else if (command.compare("eraseAtPoints") == 0) {
                toolMode = ToolModeEraser;
            }
            
            m_toolMode = toolMode;

            bool changed = false;
            int numPoints =  pointValues.size();
            for (int i = 0; success && i < numPoints; i++)
            {
                CBVerbose("getting point, ");
                
                cv::Point point = JSONHelper::getCVPointJSONValue(pointValues[i]);
                
                CBVerbose("brush at point %d of %d: (%d, %d)", (i + 1), pointValues.Size(), point.x, point.y);
                cbpipe::TouchPoint touchPoint;
                touchPoint.toolMode = toolMode;
                touchPoint.origin = point;
                
                if (i==0) {
                    touchPoint.step = TouchStepBegan;
                    CBVerbose(" - began " );
                }
                
                if (i==numPoints-1) {
                    touchPoint.step = TouchStepEnded;
                    CBVerbose(" - ended ");
                    
                } else if (i > 0) {
                    touchPoint.step = TouchStepMoved;
                    CBVerbose(" - moved ");
                }
                
                m_scene->getSelectedAsset()->touchedAtPoint(touchPoint);
                
                needsRefresh = true;
            }
            //painter.setBrushTapFillEnabled(brushTapEnabled);
            
            CBLog("%s FINISHED", command.c_str());

            return true;
        }
        
        errorMessage = string_sprintf("Command '%s' not found.", command.c_str());
        return false;
    }
    
    Json::Value saveAndRender() {
        Json::Value jsonData = nullptr;
        return jsonData;
    }
    
    CBServer *m_parent;
    std::string m_sceneID;
    
    std::shared_ptr<CBP_RenderingEngine> m_renderingPipeline;
    
    std::shared_ptr<CBAR_CallbackI> m_callback;
    std::shared_ptr<CBAR_MaskedAssetCallbackI> m_assetCallback;
    std::shared_ptr<CBAR_Scene> m_scene;
    bool m_needsScenePreparation = false;
    
    ToolMode m_toolMode = ToolModePaintbrush;
    double m_brushSize = 20;
};

CBServer::CBServer() : CBAR_Client()
{
    m_pImpl = std::unique_ptr<Impl>(new Impl(this));
}

CBServer::~CBServer() {
    //int uses = m_pImpl->m_renderingPipeline.use_count();
    //printf("Uses = %lu");
    //delete m_pImpl->m_renderingPipeline.get();
}

ToolMode CBServer::getToolMode() {
    return m_pImpl->m_toolMode;
}

void CBServer::setToolMode(ToolMode toolMode) {
    m_pImpl->m_toolMode = toolMode;
}

double CBServer::getBrushRadius() {
    return m_pImpl->m_brushSize;
}

void CBServer::setBrushRadius(double brushSize) {
    m_pImpl->m_brushSize = brushSize;
}

std::string CBServer::getCurrentSceneID() {
    return m_pImpl->m_sceneID;
}

void CBServer::setCurrentSceneID(const std::string &sceneID) {
    m_pImpl->m_sceneID = sceneID;
}

#pragma mark view CBARServerCallback
std::string CBServer::executeJSON(const std::string &json) {
    return m_pImpl->executeJSON(json);
}

void CBARServerCallback::historyChanged(std::shared_ptr<cbpipe::UndoState> undo, bool forward) {
    
}

void CBARServerCallback::colorsCallback(std::vector<cbar::ColorInfo>&colors) {
    
}

void CBARServerCallback::exposureCallback(cv::Point2f exposurePoint) {
    
}

void CBARServerCallback::focusCallback(cv::Point2f focusPoint) {
    
}

#pragma mark CBARServerAssetCallback

void CBARServerAssetCallback::attachedToScene() {
    
}

void CBARServerAssetCallback::detachedFromScene() {
    
}

void CBARServerAssetCallback::touchedAtPoint(cbpipe::TouchPoint touch) {
    
}

void CBARServerAssetCallback::rotatedBy(cbpipe::TouchPoint touch) {
    
}
