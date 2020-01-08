//
//  ProductColor+Extended.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import Alamofire

extension ProductColor {
    
    convenience init (_ json:Dictionary<String, AnyObject>) {
        self.init()
        
        self.jsonString = json.jsonString
        self.code = json["UniqueId"] as! String
        self.name = json["SellingColorName"] as! String
        //self.thumbnailPath = DataSource.getThumbnailPath(self.code)
    }
    
    var thumbnailUrl : URL? {
        return URL(string: thumbnailPath)
    }
    
    var product:Product{
        get {
            return parents[0]
        }
    }
    
    public var defaultVariation: ProductVariation {
        get {
            let variation = ProductVariation(self)
            variation.code = self.code
            variation.name = self.name
            
            return variation
        }
    }
    
    var jsonCommand:String {
        let variations = self.variations.count > 0 ? Array(self.variations) : [self.defaultVariation]
        return "{\"name\":\"\(name)\", \"variations\":[\(variations.compactMap({$0.jsonCommand}).joined(separator:","))]}"
    }
} 

