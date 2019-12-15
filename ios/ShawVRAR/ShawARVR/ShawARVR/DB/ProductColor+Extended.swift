//
//  ProductColor+Extended.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

extension ProductColor {
    
    convenience init (_ json:Dictionary<String, AnyObject>) {
        self.init()
        
        self.jsonString = json.jsonString
        self.code = json["UniqueId"] as! String
        self.name = json["SellingStyleName"] as! String
        self.thumbnailPath = DataSource.getThumbnailPath(self.code)
    }
    
    var thumbnailUrl : URL? {
        return URL(string: thumbnailPath)
    }
    
    var product:Product{
        get {
            return parents[0]
        }
    }
}

