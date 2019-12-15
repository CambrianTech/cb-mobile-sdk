//
//  Product+Extended.swift..swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//


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

    func sync(_ completion: @escaping () -> Void) {
        if (self.colors.count > 0) {
            return
        }
        
        let categoryID = self.category.code
        let styleNumber = self.styleNumber
        DispatchQueue.global(qos: .background).async {
            DataSource.current.loadProductColors(categoryID, styleNumber) { (colors) in
                DispatchQueue.main.async {
                    try! DataSource.current.realm.write {
                        self.colors.append(objectsIn: colors)
                    }
                    completion()
                }
            }
        }
    }
}
