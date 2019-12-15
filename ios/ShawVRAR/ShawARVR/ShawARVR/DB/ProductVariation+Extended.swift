//
//  ProductVariation+Extended.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/15/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import Foundation
import SDWebImage

extension ProductVariation {

    convenience init (_ color:ProductColor) {
        self.init()
    }
    
    private var remoteDiffusePath:URL? {
        return URL(string:DataSource.getDiffusePath(self.code))
    }
    
    private var remoteNormalPath:URL? {
        return nil
    }
    
    private var remoteRoughnessPath:URL? {
        return nil
    }
    
    var diffusePath:String? {
        if let path = remoteDiffusePath {
            return DataController.sharedInstance.getLocalImagePath(path)
        }
        return nil
    }

    var normalsPath:String? {
        if let path = remoteNormalPath {
            return DataController.sharedInstance.getLocalImagePath(path)
        }
        return nil
    }

    var roughnessPath:String? {
        if let path = remoteRoughnessPath {
            return DataController.sharedInstance.getLocalImagePath(path)
        }
        return nil
    }
    
    func loadData(progress: @escaping (Int, Int) -> Void, completion: @escaping (Bool) -> Void) {
        let paths:Array<URL> = []
        DataController.sharedInstance.getCachedImages(remotePaths: paths,
                                                      progress: progress,
                                                      completion: completion)
    }
    
    var jsonCommand:String {
        var data = ["name": self.name]
        if let path = diffusePath {
            data["diffusePath"] = path
        }
        if let path = normalsPath {
            data["normalsPath"] = path
        }
        if let path = roughnessPath {
            data["roughnessPath"] = path
        }
        return data.jsonString
    }
}
