//
//  BrandCategory.swift
//
//
//  Created by Joel Teply on 6/7/16.
//
//

import Foundation
import RealmSwift

class BrandCategory : Object {
    
    override static func ignoredProperties() -> [String] {
        return ["paint", "childPaints", "_cachedChildPaints", "texture", "childTextures", "_cachedChildTextures", "defaultCategory", "view", "_view"]
    }
    
    @objc dynamic var id = UUID().uuidString
    @objc dynamic var parentCategory: BrandCategory? = nil
    
    override class func primaryKey() -> String? { return "id"}
    
    //drawable
    @objc dynamic var assetPath = ""
    @objc dynamic var isIndoor = true
    @objc dynamic var isOutdoor = false
    @objc dynamic var name = ""
    @objc dynamic var orderIndex = 0
    @objc dynamic var displayItem: BrandItem? = nil
    @objc dynamic var red = 0
    @objc dynamic var green = 0
    @objc dynamic var blue = 0
    var type: CBAssetType?
    
    let items = List<BrandItem>()
    let subCategories = List<BrandCategory>()
}
