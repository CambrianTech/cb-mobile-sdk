#include <opencv2/opencv.hpp>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "CBServer.hpp"
#include <utility/Directory.h>
#include <utility/Diagnostics.h>
#include "base64.hpp"

#define TEST_SCRIPT "undo_redo"

void runServer(const cbar::CBConfig &cbConfig, const std::string &jsonEncodedOrFile, std::string &sceneID);

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string getEnvironmentVariable(const char *var) {
    std::string result;
    if (const char *varValue = std::getenv(var)) {
        result = varValue;
    }
    return result;
}

void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
    CBError("ERROR: Caught segfault at address %p", si->si_addr);
    //std::cerr << "Caught Segfault at address " << si->si_addr << std::endl;
    exit(-1);
}

int
main(int argc, char* argv[])
{
    struct sigaction sa;
    
    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags   = SA_SIGINFO;
    sigaction(SIGSEGV, &sa, NULL);
    
    auto start = sys_usec_time();

    cbar::CBConfig cbConfig;
    
    std::string cbRoot = getEnvironmentVariable("CB");
    
    if (cbRoot.empty()) {
        cbRoot = "/Users/joelteply/Development/cambrian/common-core";
    }

    
#if NDEBUG
    if (argc != 7) {
        std::cerr << "usage: CBClient licenseKey cbAssetsDirectory staticLoadDirectory imageUploadDirectory savePath  base64-str|file.json\n";
        exit(1);
    }
    std::string licenseKey = argv[1];
    cbConfig.cbAssetPath = argv[2];
    cbConfig.primaryAssetPath = argv[3];
    cbConfig.secondaryAssetPath = argv[4];
    cbConfig.workingPath = argv[5];
    
    //CBLog("Primary asset path: %s, secondary: %s ", cbConfig.primaryAssetPath.c_str(), cbConfig.secondaryAssetPath.c_str());
    
    std::string jsonEncodedOrFile = argv[6];
    
#else
    
    std::string licenseKey = "my_fake_license";
    
    cbConfig.cbAssetPath = string_sprintf("%s/sdk-assets/CBAssets", cbRoot.c_str());
    cbConfig.workingPath = string_sprintf("%s/scripts/cbclient", cbRoot.c_str());
    cbConfig.primaryAssetPath = string_sprintf("%s/scripts/cbclient/input", cbRoot.c_str());
    
    std::string jsonEncodedOrFile = string_sprintf("%s/tests/%s", cbConfig.primaryAssetPath.c_str(), TEST_SCRIPT);
#endif
    
    cbConfig.mode = CBOperationModeServer;
    cbConfig.loggingPath = string_sprintf("%s/image_logging", cbConfig.workingPath.c_str());
    
    //cbConfig.workingPath = string_sprintf("%s/scripts/cbclient", cbRoot.c_str());
    //cbConfig.primaryAssetPath = string_sprintf("%s/scripts/cbclient/input", cbRoot.c_str());
    
    std::string sceneID;
    if (Directory::exists(jsonEncodedOrFile.c_str())) {
        //directory of json files
        auto allJSONFiles = Directory::list(jsonEncodedOrFile.c_str(), false, true);
        std::sort( allJSONFiles.begin(), allJSONFiles.end());
        for (const auto &jsonFile : allJSONFiles) {
            if (ends_with(jsonFile, ".json")) {
                auto path = string_sprintf("%s/%s", jsonEncodedOrFile.c_str(), jsonFile.c_str());
                runServer(cbConfig, path, sceneID);
                if (!sceneID.length()) {
                    break;
                }
            }
        }
    } else {
        runServer(cbConfig, jsonEncodedOrFile, sceneID);
    }
    
//    while(1) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(100));
//        CBLog("Commands have taken %.3f seconds", seconds_elapsed(start));
//    }

    CBLog("Commands took %.3f seconds", seconds_elapsed(start));
    
    return 0;
}

void runServer(const cbar::CBConfig &cbConfig, const std::string &jsonEncodedOrFile, std::string &sceneID) {
    
    cbar::CB_Initialize(cbConfig);
    std::string json_commands;
    
    if (ends_with(jsonEncodedOrFile, ".json")) {
        CBLog("Opening file at %s", jsonEncodedOrFile.c_str());
        if (Directory::file_exists(jsonEncodedOrFile.c_str())) {
            json_commands = Directory::get_file_contents(jsonEncodedOrFile.c_str());
        } else {
            CBError("File '%s' not found.", jsonEncodedOrFile.c_str());
            exit(1);
        }
    } else {
        json_commands = base64_decode(jsonEncodedOrFile);
    }
    
    try {
        CBServer server;
        
        if (sceneID.length()) {
            server.setCurrentSceneID(sceneID);
        }
        
        std::string jsonResult = server.executeJSON(json_commands);
        
        sceneID = server.getCurrentSceneID();
        
        std::cerr << std::endl;
        std::cerr << "$RESULT$" << jsonResult << "$RESULT$";
        std::cerr << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
    } catch (const std::string& ex) {
        std::cerr << "ERROR: " << ex << std::endl;;
    } catch (...) {
        std::cerr << "UNKNOWN ERROR: "  << std::endl;
    }
    
}
