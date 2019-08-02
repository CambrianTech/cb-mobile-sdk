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
    
    @objc dynamic var specifications = "specs.htm"
    @objc dynamic var width:Float = 18
    @objc dynamic var height:Float = 72
    
    let parents = LinkingObjects(fromType: ProductCategory.self, property: "products")
    let installationMethods = List<InstallationMethod>()
    let colors = List<ProductColor>()
    
    var directoryPath:String {
        guard let category = parents.first else {
            fatalError("Product must have category!")
        }
        
        let basePath = category.directoryPath
        let directoryPath = "\(basePath)/products/\(name.lowercased().trim())"
        
        return directoryPath
    }
    
    var thumbnailPath:URL? {
        let pathString = "\(directoryPath)/Thumbnail.jpg".addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed)!
        return URL(string: pathString)
    }
    
    var specsPath:URL? {
        return URL(string: "\(directoryPath)/\(specifications)")
    }
    
    var jsonString:String {
        return "{\"name\":\"\(name)\", \"width\":\(width), \"height\":\(height)}"
    }
    
    class func first() -> Product? {
        guard let realmResults = DataController.sharedInstance.productContext?.objects(Product.self) else { return nil }
        return realmResults.first
    }
}

