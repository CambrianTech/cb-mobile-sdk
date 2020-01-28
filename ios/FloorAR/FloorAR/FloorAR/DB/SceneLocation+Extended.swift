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
        self.thumbnailName = json["thumbnail"] as! String
        self.previewName = json["preview"] as! String
    }
    
    var baseUrl : URL? {
        return URL(string: DataSource.sceneBaseUrl.absoluteString + "/" + self.basePath)
    }
    
    var thumbnailUrl : URL? {
        if let path = baseUrl {
            return URL(string: path.absoluteString + "/" + thumbnailName)
        }
        return nil
    }
    
    var previewUrl : URL? {
        if let path = baseUrl {
            return URL(string: path.absoluteString + "/" + previewName)
        }
        return nil
    }
}
