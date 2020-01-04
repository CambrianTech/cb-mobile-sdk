//
//  ProductCategory.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

class ProductCategory: CBDataObject {
    
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
    
    private static var _shared = ProductCategory()
    static var shared:CBDataObject {
        get {
            return _shared
        }
    }
    
    static func all() -> [ProductCategory] {
        let realmResults = DataSource.current.realm.objects(ProductCategory.self)
        return Array(realmResults);
    }
    
    func needsUpdate() -> Bool {
        let objects = ProductCategory.all()
        return objects.count == 0
    }
    
    func getDataUrl() -> URL {
        return Bundle.main.url(forResource: DataSource.ppiJsonPath, withExtension: nil)!
    }
    
    func parseObjects(data: Dictionary<String, AnyObject>) {
        
    }
}
