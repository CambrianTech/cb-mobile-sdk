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
    
    //drawable
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var orderIndex = 0
    
    var directoryPath:String {
        let basePath = parents.first?.directoryPath ?? DataController.sharedInstance.assetPath
        let directoryPath = "\(basePath)/categories/\(name.lowercased().trim())"
        
        return directoryPath
    }
    
    var thumbnailPath:URL? {
        let pathString = "\(directoryPath)/Thumbnail.jpg".addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed)!
        return URL(string: pathString)
    }
    
    let parents = LinkingObjects(fromType: ProductCategory.self, property: "categories")
    
    let categories = List<ProductCategory>()
    let products = List<Product>()
}
