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
    var color:ProductColor {
        get {
            if let color = _color {
                return color
            }
            return parents[0]
        }
    }
    
    public var remoteDiffusePath:URL? {
        return URL(string:DataSource.getDiffusePath(self.code, ppi: self.color.product.ppi))
    }
    
    public var remoteNormalPath:URL? {
        return nil
    }
    
    public var remoteRoughnessPath:URL? {
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
        var paths:Array<URL> = []
        if let url = remoteDiffusePath {
            paths.append(url)
        }
        if let url = remoteNormalPath {
            paths.append(url)
        }
        if let url = remoteRoughnessPath {
            paths.append(url)
        }
        DataController.sharedInstance.getCachedImages(remotePaths: paths,
                                                      progress: progress,
                                                      completion: completion)
    }
    
    var jsonCommand:String {
        var data = ["name": self.name]
        data["diffusePath"] = diffusePath ?? ""
        data["normalsPath"] = normalsPath ?? ""
        data["roughnessPath"] = roughnessPath ?? ""
        return data.jsonString
    }
}
