//
//  BrandCategory.swift
//
//
//  Created by Joel Teply on 6/7/16.
//
//

import Foundation
import RealmSwift

class BrandCategory : SwatchObject {
    
    @objc dynamic var parentCategory: BrandCategory? = nil
    
    override class func primaryKey() -> String? { return "id"}
    
    //drawable
    @objc dynamic var isIndoor = true
    @objc dynamic var isOutdoor = false
    @objc dynamic var displayItem: BrandItem? = nil
    
    var type: CBAssetType?
    
    let items = List<BrandItem>()
    let subCategories = List<BrandCategory>()
}
