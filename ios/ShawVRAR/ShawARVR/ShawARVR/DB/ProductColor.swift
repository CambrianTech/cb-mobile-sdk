//
//  ProductStyle.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

class ProductColor: Object {
    override static func ignoredProperties() -> [String] {
        return ["directoryPath", "thumbnailImage", "jsonString"]
    }
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    //drawable
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var orderIndex = 0
    @objc dynamic var thumbnailPath:String = ""
    @objc dynamic var jsonString:String = ""
    
    let parents = LinkingObjects(fromType: Product.self, property: "colors")
    let variations = List<ProductVariation>()
    
    var directoryPath:String?
    
    var jsonCommand:String {
        let variationsJson:String = "[\(self.variations.compactMap({$0.jsonCommand}).joined(separator:","))]"
        return "{\"name\":\"\(name)\", \"variations\":\(variationsJson)}"
    }
}
