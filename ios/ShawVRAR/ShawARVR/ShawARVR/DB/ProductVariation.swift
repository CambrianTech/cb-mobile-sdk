//
//  ProductVariation.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/24/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import Foundation
import RealmSwift
import SDWebImage

class ProductVariation: Object {
    override static func ignoredProperties() -> [String] {
        return ["directoryPath", "thumbnailImage", "jsonString"]
    }
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    //drawable
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var orderIndex = 0
    
    let parents = LinkingObjects(fromType: ProductColor.self, property: "variations")
}
