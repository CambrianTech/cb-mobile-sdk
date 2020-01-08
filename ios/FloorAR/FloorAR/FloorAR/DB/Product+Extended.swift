//
//  Product+Extended.swift..swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//


import Alamofire

extension Product {
    
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
    
    var category:ProductCategory {
        get {
            return parents[0]
        }
    }
    
    var jsonData:[String: AnyObject] {
        get {
            return self.jsonString.jsonData
        }
    }
    
    var styleNumber:String {
        get {
            return self.jsonData["SellingStyleNbr"] as! String
        }
    }
    
    var ppi:Int {
        get {
            return DataSource.current.getImagePPI(self.code)
        }
    }
    
    var dpcm:Float {
        get {
            return Float(self.ppi) / 2.54
        }
    }
    
    var jsonCommand:String {
        var data = Dictionary<String, AnyObject>()
        data["name"] = self.name as NSString
        data["dpcm"] = NSNumber(value:self.dpcm)
        return data.jsonString
    }
    
    var defaultColor:ProductColor? {
        get {
            return self.colors.first { (color) -> Bool in
                return color.code == self.code
            }
        }
    }
}
