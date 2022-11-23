//
//  CB_ARScene.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBAR_Scene.hpp"
#include <cbar/CBAR_VideoFrame.hpp>

#include <cbar/pipeline/rendering/rendering.h>
#include <utility/JSONHelper.h>
#include <cbcommon/CB_Config.h>
#include <utility/Directory.h>
#include <utility/Diagnostics.h>
#include <imaging/Imaging.h>
#include <cbar/pipeline/pipeline.h>

using namespace cbpipe;

namespace cbscene {
    
    static CBMutex lastInstanceMutex;
    static CBAR_Scene *lastInstance;
    
    struct CBAR_Scene::Impl
    {
        Impl(CBAR_Scene *scene, std::shared_ptr<cbscene::CBAR_SceneCallbackI> callback, const std::string &sceneID, const void *boundObject)
            : m_scene(scene), m_callback(callback), m_sceneID(sceneID), m_boundObject(boundObject)
        {
            lastInstanceMutex.lock();
            if (auto lastScene = lastInstance) {
                lastScene->clearAssets();
            }
            lastInstanceMutex.unlock();
            
            if (sceneID.empty()) m_sceneID = newUUID();
            
            m_lights.resize(4);
            
            //+x backwards, -x forwards
            //+y goes left, -y goes right
            //+z up, -z down
            
            //"large overhead light
            m_lights[0].type = LightingTypeIncandescent;
            m_lights[0].position = cv::Point3f(0,0,100.0);
            m_lights[0].radius = 200;
            //m_lights[0].radius = 0.1;//turn off
            m_lights[0].attenuation = 0.7;
           
            m_lights[1].type = LightingTypeDaylight;
            m_lights[1].position = cv::Point3f(-5.0,-2.0,5.0);
            m_lights[1].radius = 7;
            m_lights[1].attenuation = 0.4;
            
            m_lights[2].type = LightingTypeIncandescent;
            m_lights[2].position = cv::Point3f(2.0,-7.0,15.0);
            m_lights[2].radius = 10;
            m_lights[2].radius = 0.1;//turn off
            
            m_lights[3].type = LightingTypeDaylightEvening;
            m_lights[3].position = cv::Point3f(1.5,2.0,3000.0);
            m_lights[3].radius = 3;
            m_lights[3].radius = 0.1;//turn off
            
            update_light_matrices();
            
            lastInstanceMutex.lock();
            lastInstance = m_scene;
            lastInstanceMutex.unlock();
            
            //preload with typical floor:
            //m_worldTransform << ;
                        
            m_worldTransform.data()[0] = -0.902997553;
            m_worldTransform.data()[1] = 0.100842819;
            m_worldTransform.data()[2] = -0.417643785;
            m_worldTransform.data()[3] = -0;
            m_worldTransform.data()[4] = -0.00826357398;
            m_worldTransform.data()[5] = 0.967808842;
            m_worldTransform.data()[6] = 0.251550615;
            m_worldTransform.data()[7] = 0;
            m_worldTransform.data()[8] = 0.429566443;
            m_worldTransform.data()[9] = 0.230600804;
            m_worldTransform.data()[10] = -0.873095631;
            m_worldTransform.data()[11] = -0;
            m_worldTransform.data()[12] = -0.0768787488;
            m_worldTransform.data()[13] = -0.530504763;
            m_worldTransform.data()[14] = 0.486441553;
            m_worldTransform.data()[15] = 1;
        }
        ~Impl() {
            lastInstanceMutex.lock();
            lastInstance = nullptr;
            lastInstanceMutex.unlock();
            
            clear_assets();
        }
        
        CBAR_Scene *m_scene = 0;
        std::string m_sceneID;
        const void *m_boundObject = 0;
        std::string m_selectedAssetID;
        std::shared_ptr<cbscene::CBAR_SceneCallbackI> m_callback;
        LightingType m_lighting = LightingTypeNone;
        
        Eigen::Matrix4f m_worldTransform = Eigen::Matrix4f::Identity();
        Eigen::Matrix4f m_cameraProjection = Eigen::Matrix4f::Identity();
        Eigen::Matrix4f m_displayTransform = Eigen::Matrix4f::Identity();
        
        std::vector<cbar::structured_light> m_lights;
        
        std::map<std::string, std::string> m_userData;
        
        cv::Mat_<float> m_lightPositions;
        cv::Mat_<float> m_lightColors;

        cv::Size saveSize;
        
        bool m_has6DOF = false;

        cbar::CBAR_VideoFramePtr m_capture;
        int64_t m_frameLoadIndex = -1;
        CBMutex m_captureMutex;
        
        CBMutex m_assetMutex;
        std::map<std::string, std::shared_ptr<cbscene::CBAR_Asset>> m_assets;
        
        std::shared_ptr<cbscene::CBAR_Asset> get_selected_asset() {
            std::shared_ptr<cbscene::CBAR_Asset> result;

            std::lock_guard<CBMutex> lockGuard(m_assetMutex);
            if (m_assets.size() && !m_selectedAssetID.empty()) {
                result = m_assets[m_selectedAssetID.c_str()];
                //CBLog("Grabbing assetID %s - %s", m_assets.begin()->first.c_str(), m_assets.begin()->second == nullptr ? "is null" : "is non-null");
            } else {
                //CBLog("Asset '%s' not found. Has %d assets", m_selectedAssetID.c_str(), m_assets.size());
            }
            return result;
        }
        
        const std::map<std::string, std::shared_ptr<cbscene::CBAR_Asset>> get_assets() {
            m_assetMutex.lock();
            auto copy = m_assets;
            m_assetMutex.unlock();
            return copy;
        }
        
        bool set_selected_asset(std::shared_ptr<cbscene::CBAR_Asset>asset) {

            bool hasBeenAdded = false;
            {std::lock_guard<CBMutex> lockGuard(m_assetMutex);
                auto assetID = asset->getAssetID();
                auto it = m_assets.find(assetID);
                hasBeenAdded = (it != m_assets.end());
            }
            
            if (!hasBeenAdded) {
                append_asset(asset);
            }
            
            return set_selected_asset_id(asset->getAssetID());
        }
        
        bool set_selected_asset_id(const std::string &assetID) {
            if (m_selectedAssetID == assetID) return true;
            
            auto it = m_assets.find(assetID.c_str());
            if (it != m_assets.end()) {
                if (assetID == m_selectedAssetID) {
                    m_assets[m_selectedAssetID.c_str()]->setIsSelected(false);
                }
                
                m_selectedAssetID = assetID;
                it->second->setIsSelected(true);
                CBLog("Selected asset is: %s\n", m_selectedAssetID.c_str());
                return true;
            }
            m_selectedAssetID = std::string();
            return false;
        }
        
        int get_asset_count(CBAR_Asset::asset_type assetType) {
            
            int count = 0;
            std::lock_guard<CBMutex> lockGuard(m_assetMutex);
            for (auto &iter : m_assets) {
                if (assetType == iter.second->getType()) {
                    count ++;
                }
            }
            return count;
        }
        
        bool can_append_asset(CBAR_Asset::asset_type assetType) {
            
            int count = get_asset_count(assetType);
            
            switch (assetType) {
                case CBAR_Asset::asset_type_paint:
                    return count < 4;
                case CBAR_Asset::asset_type_floor:
                    return count < 1;
                    case CBAR_Asset::asset_type_model:
                    return count < 20;
                default:
                    return true;
                    break;
            }
        }
        
        bool append_asset(std::shared_ptr<cbscene::CBAR_Asset> asset) {
            asset->setScene(m_scene);

            {std::lock_guard<CBMutex> lockGuard(m_assetMutex);
                std::string assetID = asset->getAssetID();
                m_assets[assetID] = asset;
            };

            m_callback->assetAppended(asset);
            
            CBLog("Appended asset %s. Now %lu assets\n", asset->getAssetID().c_str(), m_assets.size());
            
            //set selected
            set_selected_asset(asset);
            
            return true;
        }
        
        bool remove_asset(const std::string &assetID) {
            bool removed = false;
            std::string newSelectedAssetID;

            {std::lock_guard<CBMutex> lockGuard(m_assetMutex);
                auto it = m_assets.find(assetID.c_str());
                if (it != m_assets.end()) {
                    removed = true;
                
                    if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
                        auto asset = m_assets[assetID.c_str()];
                        asset->detach(renderer.get());
                    }
                
                    m_callback->assetRemoved(it->second);
                    
                    m_assets.erase (it);
                    newSelectedAssetID = m_assets.empty() ? std::string() : m_assets.begin()->first;
                }
            };
 
            if (removed) {
                if (m_selectedAssetID == assetID) {
                    //if we deleted the selected asset, select the first available.
                    set_selected_asset_id(newSelectedAssetID);
                }
                CBLog("Removed asset %s. Now %lu assets\n", assetID.c_str(), m_assets.size());
            } else {
                CBLog("Asset %s was not found.\n", assetID.c_str());
            }
            
            return removed;
        }
        
        void clear_assets() {
            for (auto it : m_assets) {
                it.second->destroyRenderer();
                m_callback->assetRemoved(it.second);
            }
            m_assets.clear();
        }
        
        void save_to_directory(const std::string &_location, bool compressed, std::string &finalPath, std::string &projectJSON) {
            
            std::string location;
            if (_location.rfind("/", 0) == 0) {
                location = _location;
            } else {
                location = cbar::getWorkingAssetPath(string_sprintf("%s/%s", _location.c_str(), m_scene->getSceneID().c_str()));
            }
            
            // create
            m_directoryPath = location; // string_sprintf("%s/%s", location.c_str(), m_sceneID.c_str());
            Directory::mkpath(m_directoryPath.c_str());
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return;
            
            bool appearanceChanged = false;
            if (m_frameLoadIndex != m_capture->frameIndex && renderer) {
                appearanceChanged = true;
                //only save if changed
                cv::Mat rgb;
                renderer->fitImageToOutput(m_capture->getRGBImage(), rgb);
                
                std::string baseImagePath = string_sprintf("%s/scene.png", m_directoryPath.c_str());
                renderer->writePNG(baseImagePath, rgb);
            }
            
            //now save each asset
            {std::lock_guard<CBMutex> lockGuard(m_assetMutex);
                for (auto &iter : m_assets) {
                    std::string assetDirectory = string_sprintf("%s/%s", m_directoryPath.c_str(), iter.second->getAssetID().c_str());
                    appearanceChanged |= iter.second->saveToDirectory(assetDirectory);
                }
            };

            if (appearanceChanged) {
                //preview
                std::string previewPath = string_sprintf("%s/preview.png", m_directoryPath.c_str());
                renderer->getCallback()->saveScreenshot(previewPath, false, 100);
            }
            
            //save state
            std::vector<std::string> validDirectories;
            save_states(m_undoStatesMutex, m_undoStates, validDirectories);
            save_states(m_redoStatesMutex, m_redoStates, validDirectories);
            prune_state_directories(validDirectories);
            
            projectJSON = save_json_document(m_directoryPath);
            
            if (compressed) {
                std::string zipPath = string_sprintf("%.zip", location.c_str());
                Directory::zipDirectoryContents(location, zipPath);
                finalPath = zipPath;
            } else {
                finalPath = location;
            }
            
            //make it relative to the working path if applicable
            if (!finalPath.compare(0, cbar::getCBConfig().workingPath.size(), cbar::getCBConfig().workingPath)) {
                finalPath = finalPath.substr(1 + cbar::getCBConfig().workingPath.length());
            }
        }
        
        std::string save_json_document(const std::string &directoryPath) {
            // Create and save json document
            Json::Value rootNode;
            
            // API Version
            rootNode["version"] = API_VERSION;
            save_json_state(rootNode);
            
            //save json data
            std::string jsonPath = string_sprintf("%s/scene_data.json", directoryPath.c_str());
            
            std::string jsonStr = JSONHelper::jsonToString(rootNode);
            
            std::ofstream jsonFile(jsonPath.c_str());
            jsonFile << rootNode;
            jsonFile.close();
            
            if (!Directory::file_exists(jsonPath.c_str())) {
                CBError("json file %s does not exist", jsonPath.c_str());
            }
            
            return jsonStr;
        }
        
        std::string load_from_directory(const std::string &_location, int outputRotation, const Json::Value* sceneNode) {
            
            std::string location;
            if (_location.rfind("/", 0) == 0) {
                location = _location;
            } else {
                location = cbar::getWorkingAssetPath(_location);
            }
            
            std::string directoryPath = location;
            
            //if compressed, decompress it
            if (Directory::get_file_extension(location) == "zip") {
                directoryPath = location.substr(0, location.length()-4);
                Directory::remove_directory(directoryPath.c_str(), true);
                Directory::unzipToPath(location, directoryPath);
            }
            
            m_sceneID = Directory::get_filename(directoryPath);
            
            //load main image first
            std::string baseImagePath = string_sprintf("%s/scene.png", directoryPath.c_str());
            cv::Mat rgbaImage = cv::imread(baseImagePath, cv::IMREAD_UNCHANGED);
            if (rgbaImage.channels() == 3) {
                cv::cvtColor(rgbaImage, rgbaImage, CV_RGB2RGBA);
            }
            
            ImageProcessing::rotate_image_90n(rgbaImage, rgbaImage, outputRotation);
            
            cbar::CBAR_VideoFramePtr frame = new cbar::CBAR_VideoFrame(rgbaImage, outputRotation);
            m_scene->setVideoFrame(frame);
            
            if (sceneNode) {
                load_json_state(*sceneNode);
            }
            else {
                std::string jsonPath = string_sprintf("%s/scene_data.json", directoryPath.c_str());
                                
                if (!Directory::file_exists(jsonPath.c_str())) {
                    CBError("json file %s does not exist", jsonPath.c_str());
                    return "";
                }
                
                std::string errors;
                Json::Value scnNode;
                if (!JSONHelper::parseJSONFile(jsonPath, scnNode)) {
                    CBError("Failed to parse json file: %s", &errors);
                    return "";
                }
                load_json_state(scnNode);
            }

            //now load each asset
            auto assetsCopy = get_assets();
            
            for (auto iter : assetsCopy) {
                std::string assetDirectory = string_sprintf("%s/%s", directoryPath.c_str(), iter.second->getAssetID().c_str());
                iter.second->loadFromDirectory(assetDirectory, outputRotation);
            }
            
            m_directoryPath = directoryPath;
            
            return directoryPath;
        }
        
        std::string m_directoryPath;
        
        inline std::string get_state_path(std::shared_ptr<cbpipe::UndoState> state) {
            return string_sprintf("%s/state/%s", m_directoryPath.c_str(), state->uuid.c_str());
        }
        
        void prune_state_directories(const std::vector<std::string> &validDirectories) {

            //now prune invalid directories
            std::string rootPath = string_sprintf("%s/state", m_directoryPath.c_str());
            
            auto allDirs = Directory::list(rootPath.c_str(), true, false);
            for (const auto &dir : allDirs) {
                bool found = false;
                for (const auto &compareDir : validDirectories) {
                    if (dir == compareDir) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::string fullPath = string_sprintf("%s/%s", rootPath.c_str(), dir.c_str());
                    CBLog("Removing dir %s", dir.c_str());
                    Directory::remove_directory(fullPath.c_str(), true);
                }
            }
        }
        
        void save_states(CBMutex &mutex, const std::deque<std::shared_ptr<cbpipe::UndoState>>&states, std::vector<std::string> &validDirectories) {
            
            Json::Value undoStatesNode;
            
            std::lock_guard<CBMutex> lockGuard(mutex);
            
            for (const auto &state : states) {
                CBLog("found state %s", state->uuid.c_str());
                
                std::string undoPath = get_state_path(state);
                bool directoryNeeded = false;
                
                if (!Directory::exists(undoPath.c_str())) {
                    //directory not created, therefore images are necessary to save all images.
                    for (const auto &dataItem : state->data) {
                        
                        //TODO use any key not just hard coded mask
                        //std::string key = dataItem.first;
                        
                        if (!dataItem.second.empty()) {
                            
                            char *key = new char[100];
                            memcpy(key, dataItem.first.c_str(), dataItem.first.length());
                            cv::Mat image = dataItem.second.clone();
                            Directory::mkpath(undoPath.c_str());
                            auto imagePath = string_sprintf("%s/%s.png", undoPath.c_str(), key);
                            
                            delete [] key;
                            //CBLog("Saving history mask with %d white pixels (%s)", cv::countNonZero(image), imagePath.c_str());
                            cv::imwrite(imagePath, image);
                            directoryNeeded = true;
                            
                        }
                    }
                    
                    if (directoryNeeded) {
                        validDirectories.push_back(state->uuid);
                    } else {
                        CBLog("DIR %s not needed", state->uuid.c_str());
                    }
                } else {
                    //don't mess with it, is needed
                    validDirectories.push_back(state->uuid);
                }
            }
        }
        
        void load_states(CBMutex &mutex, std::deque<std::shared_ptr<cbpipe::UndoState>>&states) {
            std::lock_guard<CBMutex> lockGuard(mutex);
            for (std::shared_ptr<cbpipe::UndoState> state : states) {
                std::string undoPath = get_state_path(state);
                if (Directory::exists(undoPath.c_str())) {
                    for (auto &dataItem : state->data) {
                        //std::string key = "mask"; //dataItem.first;
                        if (dataItem.second.empty()) {
                            auto imagePath = string_sprintf("%s/mask.png", undoPath.c_str());
                            cv::Mat mask = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
                            //CBLog("Loaded history mask with %d white pixels (%s)", cv::countNonZero(mask), imagePath.c_str());
                            dataItem.second = mask;
                        }
                    }
                }
            }
        }
        
        bool save_json_state(Json::Value &sceneNode) {
            
            bool appearanceChanged = false;
            Json::Value userDataValue;
            
            for (auto &iter : m_userData) {
                userDataValue[iter.first] = iter.second;
            }
            sceneNode["userData"] = userDataValue;
            
            sceneNode["frameIndex"] = m_capture->frameIndex;
            sceneNode["lighting"] = int(m_lighting);
            sceneNode["selectedAsset"] = m_selectedAssetID;
            
            Json::Value worldTransform;
            JSONHelper::fillJSONArray(worldTransform, m_worldTransform.data(), m_worldTransform.size());
            sceneNode["worldTransform"] = worldTransform;
            
            Json::Value projectionMatrix;
            JSONHelper::fillJSONArray(projectionMatrix, m_cameraProjection.data(), m_cameraProjection.size());
            sceneNode["projectionMatrix"] = projectionMatrix;
            
            //assets
            Json::Value assetsArray;
            {std::lock_guard<CBMutex> lockGuard(m_assetMutex);
                for (auto &iter : m_assets) {
                    Json::Value assetNode;
                    appearanceChanged |= iter.second->saveJSONState(m_scene, assetNode);
                    assetsArray.append(assetNode);
                }
            };
            sceneNode["assets"] = assetsArray;
            
            //Undo/redo
            Json::Value historyNode;

            { //UNDO
                Json::Value undoStatesNode;
                std::lock_guard<CBMutex> lockGuard(m_undoStatesMutex);
                CBLog("Saving %lu undo state%s", m_undoStates.size(), (m_undoStates.size() == 1 ? "" : "s"));
                for (const auto &undoState : m_undoStates) {
                    undoStatesNode.append(create_json_history_item(undoState));
                }
                historyNode["undo"] = undoStatesNode;
            };
            
            { //REDO
                Json::Value redoStatesNode;
                std::lock_guard<CBMutex> lockGuard(m_redoStatesMutex);
                CBLog("Saving %lu redo state%s", m_redoStates.size(), (m_redoStates.size() == 1 ? "" : "s"));
                for (const auto &redoState : m_redoStates) {
                    redoStatesNode.append(create_json_history_item(redoState));
                }
                historyNode["redo"] = redoStatesNode;
            };
            
            sceneNode["history"] = historyNode;
            
            return appearanceChanged;
        }
        
        Json::Value create_json_history_item(std::shared_ptr<cbpipe::UndoState> item) {
            Json::Value undoNode;

            CBLog("State UUID: %s", item->uuid.c_str());
            
            undoNode["target"] = int(item->target);
            undoNode["change"] = int(item->change);
            undoNode["assetID"] = item->assetID;
            undoNode["uuid"] = item->uuid;
            
            Json::Value dataNode;

            //ndk string issues
//            for (const auto &dataItem : item->data) {
//                dataNode.append(dataItem.first);
//            }

            if (!item->data.empty()) {
                dataNode.append("mask");
            }

            undoNode["data"] = dataNode;
            
            Json::Value userInfoNode;
            for (const auto &userInfo : item->stateInfo) {
                undoNode[userInfo.first] = userInfo.second;
            }
            undoNode["stateInfo"] = userInfoNode;
            
            return undoNode;
        }
        
        std::shared_ptr<cbpipe::UndoState> get_json_history_item(const Json::Value &undoNode) {
            
            std::shared_ptr<cbpipe::UndoState> state = std::shared_ptr<cbpipe::UndoState>(new UndoState);
            
            if (undoNode.isMember("target")) {
                state->target = (undo_target) undoNode["target"].asInt();
            }
            
            if (undoNode.isMember("change")) {
                state->change = (undo_change) undoNode["change"].asInt();
            }
            
            if (undoNode.isMember("assetID")) {
                state->assetID = undoNode["assetID"].asString();
            }
            
            if (undoNode.isMember("uuid")) {
                state->uuid = undoNode["uuid"].asString();
            }
            
            if (undoNode.isMember("data")) {
                const Json::Value &dataNode = undoNode["data"];
                
                for (int i=0; i<dataNode.size(); i++) {
                    const Json::Value& data = dataNode[i];
                    state->data[data.asString()] = cv::Mat();
                }
            }

            if (undoNode.isMember("stateInfo")) {
                const Json::Value &stateNode = undoNode["stateInfo"];
                
                for (Json::Value::const_iterator itr = stateNode.begin() ; itr != stateNode.end() ; itr++ ) {
                    state->stateInfo[itr.key().asString().c_str()] = itr->asString();
                }
            }            
            return state;
        }
        
        void load_json_state(const Json::Value& sceneNode) {
            
            double sdkVersion = API_VERSION;
            
            if (sceneNode.isMember("version")) {
                sdkVersion = sceneNode["version"].asDouble();
            }
            
            CBWrite("Loading scene data for SDK version %3f", sdkVersion);
            const Json::Value& userDataValues = sceneNode["userData"];
            
            for (Json::Value::const_iterator itr = userDataValues.begin() ; itr != userDataValues.end() ; itr++ ) {
                m_userData[itr.key().asString()] = itr->asString();
            }
            
            m_frameLoadIndex = sceneNode["frameIndex"].asInt();
            
            if (sceneNode.isMember("lighting")) {
                const Json::Value& lightingNode = sceneNode["lighting"];
                if (!lightingNode.isNull()) {
                    m_lighting = static_cast<LightingType>(lightingNode.asInt());
                }
            }
            
            if (sceneNode.isMember("selectedAsset")) {
                const Json::Value& selectedAssetValue = sceneNode["selectedAsset"];
                if (!selectedAssetValue.isNull()) {
                    set_selected_asset_id(selectedAssetValue.asString());
                }
            }
            
            if (sceneNode.isMember("worldTransform")) {
                const Json::Value& wtValue = sceneNode["worldTransform"];
                if (!wtValue.isNull() && wtValue.isArray()) {
                     JSONHelper::parseJSONArray(wtValue, m_worldTransform.data());
                }
            }
            
            //causing an issue in BGFX
            if (sceneNode.isMember("projectionMatrix")) {
                const Json::Value& projValue = sceneNode["projectionMatrix"];
                if (!projValue.isNull() && projValue.isArray()) {
                    JSONHelper::parseJSONArray(projValue, m_cameraProjection.data());
                }
            }
            
            if (sceneNode.isMember("assets")) {
                const Json::Value& assetsArrayNode = sceneNode["assets"];

                if (assetsArrayNode.isArray()) {
                    
                    for (Json::Value::ArrayIndex i = 0; i != assetsArrayNode.size(); i++) {
                        const Json::Value& assetNode = assetsArrayNode[i];
                        
                        if (assetNode.isMember("assetID") && assetNode.isMember("type")) {
                            CBAR_Asset::asset_type assetType = (CBAR_Asset::asset_type) assetNode["type"].asInt();
                            std::string assetID = assetNode["assetID"].asString();
                            //CBLog("Got assetID=%s", assetID.c_str());
                            
                            //JNI has issue with this assetID string getting deallocated when calling lambda function
                            if (m_assets.count(assetID)) {
                                m_assets[assetID]->loadJSONState(m_scene, assetNode);
                            } else {
                                //new asset.
                                std::string _assetID = assetID;
                                if (auto assetPtr = m_callback->generateAsset(assetType, _assetID)) {
                                    CBLog("asset use count=%d", assetPtr.use_count());
                                    m_assetMutex.lock();
                                    m_assets[assetID] = assetPtr;
                                    m_assetMutex.unlock();
                                    
                                    assetPtr->loadJSONState(m_scene, assetNode);
                                }
                            }
                            
                            //CBLog("Loaded assetID %s, isNull=%d", m_assets.begin()->first.c_str(), m_assets.begin()->second == nullptr);
                        }
                    }
                    if (!m_selectedAssetID.length() && m_assets.size()) {
                        m_selectedAssetID = m_assets.rbegin()->first;
                        m_assets.rbegin()->second->setIsSelected(true);
                    }
                }
            }
            
            //Undo/redo
            if (sceneNode.isMember("history")) {
                const Json::Value &historyNode = sceneNode["history"];
                
                //Undo
                if (historyNode.isMember("undo") && historyNode["undo"].isArray()) {
                    const Json::Value &undoStatesNode = historyNode["undo"];
                    
                    {std::lock_guard<CBMutex> lockGuard(m_undoStatesMutex);
                        m_undoStates.clear();
                        for (int i = 0; i < undoStatesNode.size(); i++) {
                            const Json::Value& undoNode = undoStatesNode[i];
                            const auto undoState = get_json_history_item(undoNode);
                            m_undoStates.push_back(undoState);
                        }
                    };
                }
                
                //redo
                if (historyNode.isMember("redo") && historyNode["redo"].isArray()) {
                    const Json::Value &redoStatesNode = historyNode["redo"];
                    
                    {std::lock_guard<CBMutex> lockGuard(m_redoStatesMutex);
                        m_redoStates.clear();
                        for (int i = 0; i < redoStatesNode.size(); i++) {
                            const Json::Value& redoNode = redoStatesNode[i];
                            const auto redoState = get_json_history_item(redoNode);
                            m_redoStates.push_back(redoState);
                        }
                    };
                }
            }
        }
        
        int m_maxUndoSize = 10;
        CBMutex m_undoStatesMutex;
        std::deque<std::shared_ptr<cbpipe::UndoState>> m_undoStates;
        
        int m_maxRedoSize = 10;
        CBMutex m_redoStatesMutex;
        std::deque<std::shared_ptr<cbpipe::UndoState>> m_redoStates;
        
        inline void log_states(const std::deque<std::shared_ptr<cbpipe::UndoState>> &states, const std::string &type) {
            if (states.size() == 1) {
                CBLog("There is now 1 %s state", type.c_str());
            } else {
                CBLog("There are now %lu %s states", states.size(), type.c_str());
            }
        }
        
        void append_state (std::shared_ptr<cbpipe::UndoState> state, std::deque<std::shared_ptr<cbpipe::UndoState>> &states, CBMutex& mutex, int max, bool forward) {

            if (!state->uuid.length()) {
                state->uuid = newUUID();
            }

            {std::lock_guard<CBMutex> lockGuard(mutex);
                std::string type = forward ? "undo" : "redo";
                states.push_back(state);
                //CBLog("Appending %s state %s", type.c_str(), state.uuid.c_str());
                if (states.size() > max) {
                    //CBLog("Removing %s state %s", type.c_str(), states.front().uuid.c_str());
                    states.pop_front();
                }
                
                //logStates(states, type);
            };
            
            auto renderer = CBP_RenderingEngine::sharedInstance();
            if (renderer) {
                auto callback = renderer->getCallback();
                if (callback) {
                    callback->historyChanged(state, forward);
                }
            }
        }

        void append_undo_state(std::shared_ptr<cbpipe::UndoState> state) {
            append_state(state, m_undoStates, m_undoStatesMutex, m_maxUndoSize, true);
        }
        
        void append_redo_state(std::shared_ptr<cbpipe::UndoState> state) {
            append_state(state, m_redoStates, m_redoStatesMutex, m_maxRedoSize, false);
        }
        
        std::shared_ptr<cbpipe::UndoState> getLastState(std::deque<std::shared_ptr<cbpipe::UndoState>> &states, CBMutex& mutex) {

            load_states(mutex, states);
            
            std::shared_ptr<cbpipe::UndoState> state;
            
            std::lock_guard<CBMutex> lockGuard(mutex);
            if (!states.empty()) {
                state = states.back();
                //CBLog("mmask has %d white pixels", cv::countNonZero(state.data["mask"]));
            }
            
            return state;
        }

        void remove_last_state(std::deque<std::shared_ptr<cbpipe::UndoState>> &states, CBMutex& mutex, const std::string &type) {
            std::lock_guard<CBMutex> lockGuard(mutex);
            if (!states.empty())
                states.pop_back();
            
            //logStates(states, type);
        }
        
        bool undo_redo_state(std::deque<std::shared_ptr<cbpipe::UndoState>> &states, CBMutex& mutex, bool isUndo) {
            
            int numStates = 0;
            {std::lock_guard<CBMutex> lockGuard(mutex);
                numStates = int(states.size());
            };
            
            if (!numStates) return false;
            
            std::shared_ptr<cbpipe::UndoState> undoOrRedo = getLastState(states, mutex);
            
            //CBLog("Mask has %d white pixels", cv::countNonZero(undoOrRedo.data["mask"]));
            
            std::shared_ptr<CBAR_Asset> asset = nullptr;
            {std::lock_guard<CBMutex> lockGuard(m_assetMutex);
                asset = m_assets[undoOrRedo->assetID.c_str()];
            };
            
            if (!asset || !undoOrRedo->assetID.length()) {
                return false;
            }

            auto newState = std::make_shared<cbpipe::UndoState>(*undoOrRedo);//copy
            newState->uuid = newUUID();
            asset->captureState(newState);
            
            if (isUndo) {
                //capture current state and put it in redo
                append_redo_state(newState);
            } else {
                append_undo_state(newState);
            }

            asset->reloadState(undoOrRedo);
            
            remove_last_state(states, mutex, isUndo ? "undo" : "redo");
            
            if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
                if (auto colorAnalyzer = renderer->getAnalyzerOfType<CBP_AmbienceSampler>()) {
                    colorAnalyzer->recalculate();
                }
            }

            return true;
        }

        bool undo_state() {
            return undo_redo_state(m_undoStates, m_undoStatesMutex, true);
        }
        
        bool redo_state() {
            return undo_redo_state(m_redoStates, m_redoStatesMutex, false);
        }
        
        int get_undo_size() {
            std::lock_guard<CBMutex> lockGuard(m_undoStatesMutex);
            return (static_cast<int>(m_undoStates.size()));
        }
        
        int get_redo_size() {
            std::lock_guard<CBMutex> lockGuard(m_redoStatesMutex);
            return (static_cast<int>(m_redoStates.size()));
        }

        cv::Size get_save_size() const {
            return saveSize;
        }

        void capture_to_still() {
            {std::lock_guard<CBMutex> lockGuard(m_assetMutex);
                for (auto &iter : m_assets) {
                    iter.second->captureToStill();
                }
            };
        }
        
        void going_live() {
            std::lock_guard<CBMutex> lockGuard(m_assetMutex);
            for (auto &iter : m_assets) {
                iter.second->goingLive();
            }
        }
        
        void update_light_matrices() {
            int numLights = int(m_lights.size());
            m_lightPositions = cv::Mat::ones(numLights,4, CV_32F);
            m_lightColors = cv::Mat::ones(numLights,4, CV_32F);
            
            for (int i=0; i<numLights; i++) {
                const auto &light = m_lights[i];
                auto lightMultiplier = cbpipe::CBP_RenderUtility::getLightingMultiplier(light.type);
                
                for (int j=0; j<3; j++) {
                    m_lightColors.at<float>(i,j) = lightMultiplier[j];
                }
                
                //set attentuation if needed here:
                m_lightColors.at<float>(i,3) = light.attenuation;
                
                //set positions
                m_lightPositions.at<float>(i,0) = light.position.x;
                m_lightPositions.at<float>(i,1) = light.position.y;
                m_lightPositions.at<float>(i,2) = light.position.z;
                
                m_lightPositions.at<float>(i,3) = light.radius;
            }
        }
        
        void set_world_transform(const Eigen::Matrix4f& wt) {
            
            if (m_has6DOF) {
                m_worldTransform = wt;
            } else {
                //TODO: get these actual values, y is up
                //m_worldTransform(0,3) = 0;//x
                //m_worldTransform(1,3) = -1.5;//y meters above ground
                //m_worldTransform(2,3) = 0;//z
                
                //1.5 meters high
                m_worldTransform = wt * CBP_MatrixUtil::makeRotationMatrix(M_PI_2, 0, 0)
                 * CBP_MatrixUtil::makeTranslationMatrix(0, -1.5, 0);
            }
            
        }
    };
    
    CBAR_Scene::~CBAR_Scene() {
        CBLog("DESTROYED SCENE \n");
    }
    
    CBAR_Scene::CBAR_Scene(std::shared_ptr<cbscene::CBAR_SceneCallbackI> callback, const std::string &sceneID, const void *boundObject)  {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, callback, sceneID, boundObject));
    }
    
    CBAR_Scene::CBAR_Scene(std::shared_ptr<cbscene::CBAR_SceneCallbackI> callback, const cv::Mat &bgraImage, const std::string &sceneID, const void *context)
        : CBAR_Scene::CBAR_Scene(callback, sceneID, context) {

        if (!bgraImage.empty()) {
            //Diagnostics::SaveDiagnosticImage(false, rgbaImage, "test2.png");
            cbar::CBAR_VideoFramePtr frame = new cbar::CBAR_VideoFrame(bgraImage, 0, false, false);
            m_pImpl->m_scene->setVideoFrame(frame);
        }
    }
    
    std::string& CBAR_Scene::getSceneID() {
        return m_pImpl->m_sceneID;
    }
    
    void CBAR_Scene::setSceneID(const std::string &sceneID) {
        m_pImpl->m_sceneID = sceneID;
    }
    
    std::map<std::string, std::string>& CBAR_Scene::getUserData() {
        return m_pImpl->m_userData;
    }
    
    LightingType CBAR_Scene::getLighting() {
        return m_pImpl->m_lighting;
    }
    
    void CBAR_Scene::setLighting(LightingType lighting) {
        m_pImpl->m_lighting = lighting;
        if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
            renderer->setLighting(lighting);
        }
    }
    
    const void * CBAR_Scene::getContext() const {
        return m_pImpl->m_boundObject;
    }
    
    std::string CBAR_Scene::getSelectedAssetID() const {
        return m_pImpl->m_selectedAssetID;
    }
    
    bool CBAR_Scene::setSelectedAssetID(const std::string &assetID) {
        return m_pImpl->set_selected_asset_id(assetID);
    }
    
    std::shared_ptr<cbscene::CBAR_Asset> CBAR_Scene::getSelectedAsset() {
        return m_pImpl->get_selected_asset();
    }
    
    bool CBAR_Scene::setSelectedAsset(std::shared_ptr<cbscene::CBAR_Asset>asset) {
        return m_pImpl->set_selected_asset(asset);
    }
    
    const std::map<std::string, std::shared_ptr<cbscene::CBAR_Asset>> CBAR_Scene::getAssets() {
        return m_pImpl->get_assets();
    }
    
    int CBAR_Scene::getAssetCount(CBAR_Asset::asset_type assetType) {
        return m_pImpl->get_asset_count(assetType);
    }
    
    bool CBAR_Scene::canAppendAsset(CBAR_Asset::asset_type assetType) {
        return m_pImpl->can_append_asset(assetType);
    }
    
    bool CBAR_Scene::appendAsset(std::shared_ptr<cbscene::CBAR_Asset> asset) {
        return m_pImpl->append_asset(asset);
    }
    
    bool CBAR_Scene::removeAsset(const std::string &assetID) {
        return m_pImpl->remove_asset(assetID);
    }
    
    void CBAR_Scene::clearAssets() {
        return m_pImpl->clear_assets();
    }
    
    cbar::CBAR_VideoFramePtr CBAR_Scene::getVideoFrame() {
        std::lock_guard<CBMutex> lockGuard(m_pImpl->m_captureMutex);
        return m_pImpl->m_capture;
    }
    
    void CBAR_Scene::setVideoFrame(cbar::CBAR_VideoFramePtr capture) {
        if (capture.empty() || !capture->frameSize().width) return;
        
#if DEBUG_LIGHTS
        m_pImpl->m_lights[1].position.z += 0.03;
        m_pImpl->update_light_matrices();
#endif
        
        std::lock_guard<CBMutex> lockGuard(m_pImpl->m_captureMutex);
        m_pImpl->m_capture = capture;
    }
    
    void CBAR_Scene::saveToDirectory(const std::string &location, bool compressed, std::string &finalPath, std::string &projectJSON) {
        m_pImpl->save_to_directory(location, compressed, finalPath, projectJSON);
    }
    
    void CBAR_Scene::saveJSONState(Json::Value &jsonDocument) {
        m_pImpl->save_json_state(jsonDocument);
    }
    
    std::string CBAR_Scene::loadFromDirectory(const std::string &location, int outputRotation, const Json::Value* sceneNode) {
        return m_pImpl->load_from_directory(location, outputRotation, sceneNode);
    }
    
    void CBAR_Scene::loadJSONState(const Json::Value& sceneNode) {
        m_pImpl->load_json_state(sceneNode);
    }
    
    cv::Mat CBAR_Scene::getOriginalImageAtPath(const std::string &directoryPath) {
        std::string baseImagePath = string_sprintf("%s/scene.png", directoryPath.c_str());
        cv::Mat rgbaImage = cv::imread(baseImagePath, cv::IMREAD_UNCHANGED);
        if (!rgbaImage.empty()) {
            cv::cvtColor(rgbaImage, rgbaImage, CV_RGBA2BGRA);
        }
        
        return rgbaImage;
    }
    
    cv::Mat CBAR_Scene::getPreviewImageAtPath(const std::string &directoryPath) {
        std::string baseImagePath = string_sprintf("%s/preview.jpg", directoryPath.c_str());
        cv::Mat rgbaImage = cv::imread(baseImagePath, cv::IMREAD_UNCHANGED);
        if (!rgbaImage.empty()) {
            cv::cvtColor(rgbaImage, rgbaImage, CV_RGBA2BGRA);
        }
        return rgbaImage;
    }
    
    void CBAR_Scene::appendUndoData(std::shared_ptr<cbpipe::UndoState> state) {
        {std::lock_guard<CBMutex> lockGuard(m_pImpl->m_redoStatesMutex);
            m_pImpl->m_redoStates.clear();
        };
        m_pImpl->append_undo_state(state);
    }
    
    bool CBAR_Scene::undoState() {
        return m_pImpl->undo_state();
    }
    
    bool CBAR_Scene::redoState() {
        return m_pImpl->redo_state();
    }
    
    int CBAR_Scene::getUndoSize() const {
        return m_pImpl->get_undo_size();
    }
    
    int CBAR_Scene::getMaxUndoSize() const {
        return m_pImpl->m_maxUndoSize;
    }
    
    void CBAR_Scene::setMaxUndoSize(int size) {
        m_pImpl->m_maxUndoSize = size;
    }
    
    int CBAR_Scene::getRedoSize() const {
        return m_pImpl->get_redo_size();
    }

    cv::Size CBAR_Scene::getSaveSize() const {
        return m_pImpl->get_save_size();
    }

    int CBAR_Scene::getMaxRedoSize() const {
        return m_pImpl->m_maxRedoSize;
    }
    
    void CBAR_Scene::setMaxRedoSize(int size) {
        m_pImpl->m_maxRedoSize = size;
    }
    
    void CBAR_Scene::captureToStill() {
        m_pImpl->capture_to_still();
    }
    
    void CBAR_Scene::goingLive() {
        m_pImpl->going_live();
    }
    
    const cv::Mat_<float>& CBAR_Scene::getLightPositions() const {
        return m_pImpl->m_lightPositions;
    }
    
    const cv::Mat_<float>& CBAR_Scene::getLightColors() const {
        return m_pImpl->m_lightColors;
    }

    const Eigen::Matrix4f& CBAR_Scene::getWorldTransform() const {
        return m_pImpl->m_worldTransform;
    }

    bool CBAR_Scene::hasWorldTransform() {
        return !m_pImpl->m_worldTransform.isIdentity();
    }
    
    void CBAR_Scene::setWorldTransform(const Eigen::Matrix4f& wt, bool has6DOF) {
        m_pImpl->m_has6DOF = has6DOF;
        m_pImpl->set_world_transform(wt);
    }
    
    void CBAR_Scene::setWorldTransform(const cv::Vec4f &quaterionXYZW,
                                       const cv::Vec4f &cameraPosition) {
        
        cv::Vec4f deviceCorrected = cv::Vec4f(quaterionXYZW[0],
                                              quaterionXYZW[1],
                                              quaterionXYZW[2],
                                              quaterionXYZW[3]);
        
        auto wt = cbpipe::CBP_MatrixUtil::makeRotationMatrix(deviceCorrected);
        wt(0,3) = cameraPosition[0];
        wt(1,3) = cameraPosition[1];
        wt(2,3) = cameraPosition[2];
        
        setWorldTransform(wt, false);
    }
    
    void CBAR_Scene::setCameraTransform(const Eigen::Matrix4f& ct, bool has6DOF) {
        if (!ct.isZero() && !ct.isIdentity()) {
            setWorldTransform(ct.inverse(), has6DOF);
        }
    }
    
    Eigen::Matrix4f CBAR_Scene::getCameraTransform() const {
        return m_pImpl->m_worldTransform.inverse();
    }
    
    const Eigen::Matrix4f& CBAR_Scene::getCameraProjection() const {
        return m_pImpl->m_cameraProjection;
    }
    
    void CBAR_Scene::setCameraProjection(const Eigen::Matrix4f& proj) {
        if (!proj.isZero() && !proj.isIdentity()) {
            m_pImpl->m_cameraProjection = proj;
            //std::cout << "Camera projection: \n" << proj << std::endl;
        }
    }
    
    bool CBAR_Scene::hasCameraProjection() {
        return !m_pImpl->m_cameraProjection.isIdentity();
    }
    
    void CBAR_Scene::setDisplayTransform(const Eigen::Matrix4f& dt) {
        m_pImpl->m_displayTransform = dt;
        //std::cout << "Display transform: \n" << dt << std::endl;
    }
    
    bool CBAR_Scene::has6DOF() const {
        return m_pImpl->m_has6DOF;
    }
    
    const Eigen::Matrix4f& CBAR_Scene::getDisplayTransform() const {
        return m_pImpl->m_displayTransform;
    }
};
