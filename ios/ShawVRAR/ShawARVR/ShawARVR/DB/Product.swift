//
//  Product.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

class Product: Object {
    override static func ignoredProperties() -> [String] {
        return ["directoryPath", "thumbnailImage"]
    }
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    //drawable
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var orderIndex = 0
    
    @objc dynamic var ppi:Float = 20
    @objc dynamic var width:Float = 1024
    @objc dynamic var height:Float = 1024
    @objc dynamic var thumbnailPath:String = ""
    
    let parents = LinkingObjects(fromType: ProductCategory.self, property: "products")
    let installationMethods = List<InstallationMethod>()
    let colors = List<ProductColor>()
    
    class func first() -> Product? {
        guard let realmResults = DataController.sharedInstance.productContext?.objects(Product.self) else { return nil }
        return realmResults.first
    }
    
    class func random() -> Product? {
        guard let realmResults = DataController.sharedInstance.productContext?.objects(Product.self) else { return nil }
        let index = Int (arc4random_uniform(UInt32(realmResults.count)));
        return realmResults[index];
    }
}
