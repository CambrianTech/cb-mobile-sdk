//
//  SceneLocation+Extended.swift
//  FloorAR
//
//  Created by Joel Teply on 1/2/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import Foundation

extension SceneLocation {
    
    convenience init (_ json:Dictionary<String, AnyObject>) {
        self.init()
        
        self.jsonString = json.jsonString
        self.code = json["name"] as! String
        self.name = json["displayName"] as! String
        self.basePath = json["path"] as! String
        self.thumbnailPath = json["thumbnail"] as! String
        self.previewPath = json["preview"] as! String
    }
    
    var baseUrl : URL? {
        return URL(string: DataSource.sceneBaseUrl.absoluteString + "/" + self.basePath)
    }
    
    var thumbnailUrl : URL? {
        if let path = baseUrl {
            return URL(string: path.absoluteString + "/" + thumbnailPath)
        }
        return nil
    }
    
    var previewUrl : URL? {
        if let path = baseUrl {
            return URL(string: path.absoluteString + "/" + previewPath)
        }
        return nil
    }
}
