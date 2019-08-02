//
//  BrandMaterial.swift
//
//
//  Created by Joel Teply on 6/7/16.
//
//

import Foundation
import RealmSwift

class BrandItem : Object {
    
    override static func ignoredProperties() -> [String] {
        return ["storeURL", "_view", "view", "color", "text"]
    }
    
    @objc dynamic var id: String = ""
    override class func primaryKey() -> String? { return "id"}
    
    convenience init(key: String) {
        self.init()
        self.id = key
    }
    
    @objc dynamic var parentCategory: BrandCategory? = nil
    @objc dynamic var name = ""
    @objc dynamic var storeID = ""
    @objc dynamic var orderIndex = 0
    @objc dynamic var storeLink = ""
    
    //material, model specific
    @objc dynamic var assetPath = ""
    @objc dynamic var scale: Float = 1.0
    @objc dynamic var reflectivity: Float = 5.0
    
    //paint specific
    @objc dynamic var red = 0
    @objc dynamic var green = 0
    @objc dynamic var blue = 0
    
    @objc dynamic var info: String?
    
    @objc dynamic var opacity = 255
    
    @objc dynamic var type = CBAssetType.paint
}
