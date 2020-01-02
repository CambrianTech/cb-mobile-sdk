//
//  ProductCategory.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

class ProductCategory: Object {
    override static func ignoredProperties() -> [String] {
        return []
    }
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var orderIndex = 0
    @objc dynamic var thumbnailPath:String = ""
    @objc dynamic var jsonString:String = ""
    
    let parents = LinkingObjects(fromType: ProductCategory.self, property: "categories")
    
    let categories = List<ProductCategory>()
    let products = List<Product>()
}
