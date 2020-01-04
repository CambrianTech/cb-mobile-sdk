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
        if (self == ProductCategory._shared) {
            //top level
            let objects = ProductCategory.all()
            return objects.count == 0
        }
        return self.products.count == 0
    }
    
    func getDataUrl() -> URL {
        return Bundle.main.url(forResource: DataSource.categoryJsonPath, withExtension: nil)!
    }
    
    var isTopLevel:Bool {
        get {
            return ProductCategory._shared == self
        }
    }
    
    func parseObjects(data: Dictionary<String, AnyObject>) {
        
        if (isTopLevel) {
            guard let categoriesJson = data["categories"] as? Array<AnyObject> else {
                fatalError("no source attribute in json data")
            }
            for categoryJson in categoriesJson {
                let dict = categoryJson as! Dictionary<String, AnyObject>
                let category = ProductCategory(dict)
                categories.append(category)
                try! DataSource.current.realm.write {
                    DataSource.current.realm.add(category)
                }
            }
            print("Created \(ProductCategory.all().count) categories")
        } else {
            
        }
    }
    
    func sync(_ completion: @escaping () -> Void) {
        if (self.products.count > 0) {
            completion()
        } else {
            let categoryData = self.jsonString.jsonData
            DispatchQueue.global(qos: .background).async {
                Product.loadProducts(categoryData) { (products) in
                    DispatchQueue.main.async {
                        try! DataSource.current.realm.write {
                            self.products.append(objectsIn: products)
                        }
                        completion()
                    }
                }
            }
        }
    }
}
