//
//  CBAR_Common.cpp
//  Cambrian
//
//  Created by Joel Teply on 8/27/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBAR_Common.hpp"
#include <stdio.h>
#include <utility/Threads.h>
#include <utility/Directory.h>
#include <utility/CommonUtility.h>
#include <utility/Diagnostics.h>
#include <cbar/pipeline/analysis/analysis.h>
#include <cbar/pipeline/machine-learning/machine-learning.h>

using namespace cbpipe;

namespace cbar {
    static CBConfig sharedConfig;
    
    const CBConfig& getCBConfig() {
        return sharedConfig;
    }
    
    std::string getCBAssetPath(const std::string &relativePath) {
        return string_sprintf("%s/%s", sharedConfig.cbAssetPath.c_str(), relativePath.c_str());
    }
    
    std::string getWorkingAssetPath(const std::string &relativePath) {
        return string_sprintf("%s/%s", sharedConfig.workingPath.c_str(), relativePath.c_str());
    }
    
    std::string getCompressedCBAsset(const std::string &relativePath) {
        size_t lastindex = relativePath.find_last_of(".");
        
        std::string decompressedRelativePath = relativePath.substr(0, lastindex);
        std::string decompressedPath = getWorkingAssetPath(decompressedRelativePath);
        std::string bundlePath = getCBAssetPath(decompressedRelativePath);
        
        if (Directory::file_exists(bundlePath.c_str())) {
            return bundlePath; //isn't compressed apparently
        }
        else if (Directory::file_exists(decompressedPath.c_str())) {
            return decompressedPath; //is already decompressed
        }
        return ""; //still working on it
    }
    
    std::string getUserAssetPath(const std::string &relativePath) {
        auto primaryFile = string_sprintf("%s/%s", sharedConfig.primaryAssetPath.c_str(), relativePath.c_str());
        auto secondaryFile = string_sprintf("%s/%s", sharedConfig.secondaryAssetPath.c_str(), relativePath.c_str());
        
        if (Directory::file_exists(primaryFile.c_str())) {
            return primaryFile;
        } else if (Directory::file_exists(secondaryFile.c_str())) {
            return secondaryFile;
        }
        
        return "";
    }
    
    void prepareSystem() {
        //any preloading

        
    }
    
    void CB_Initialize(CBConfig config) {
        
        sharedConfig = config;
        
        Diagnostics::Initialize(sharedConfig.loggingPath);
        Directory::mkpath(sharedConfig.workingPath.c_str());
        
        if (sharedConfig.preloadSystem) {
            prepareSystem();
        }
    }
}
