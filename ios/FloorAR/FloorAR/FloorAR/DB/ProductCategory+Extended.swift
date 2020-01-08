//
//  ProductCategory+Extended.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

extension ProductCategory {
    
    convenience init (_ json:Dictionary<String, AnyObject>) {
        self.init()
        
        self.jsonString = json.jsonString
        self.code = json["code"] as! String
        self.name = json["displayName"] as! String
    }
    
    var thumbnailUrl : URL? {
        return URL(string: thumbnailPath)
    }
}

