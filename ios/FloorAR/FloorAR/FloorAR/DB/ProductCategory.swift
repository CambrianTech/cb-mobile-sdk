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
        if (Date().days(from: self.updated) > 3) {
            return true
        }
        
        if (isTopLevel) {
            //top level
            let objects = ProductCategory.all()
            return objects.count == 0
        }
        return self.products.count == 0
    }
    
    func getDataUrl() -> URL? {
        if (isTopLevel) {
            return Bundle.main.url(forResource: DataSource.categoryJsonPath, withExtension: nil)!
        }
        return ProductCategory.buildProductDataRequest(self.jsonString.jsonData)
    }
    
    func parseObjects(data: Dictionary<String, AnyObject>, realm:Realm) {
        if (isTopLevel) {
            guard let categoriesJson = data["categories"] as? Array<AnyObject> else {
                fatalError("no source attribute in json data")
            }
            
            self.categories.removeAll()
            for categoryJson in categoriesJson {
                let dict = categoryJson as! Dictionary<String, AnyObject>
                let category = ProductCategory(dict)
                self.categories.append(category)
                realm.add(category, update: .modified)
            }
            
            print("Created \(ProductCategory.all().count) categories")
        } else {
            guard let productsJSON = data["value"] as? Array<Dictionary<String, AnyObject>> else {
                fatalError("no value attribute in json data")
            }
            self.products.removeAll()
            for productJson in productsJSON {
                let product = Product(productJson)
                realm.add(product, update: .modified)
                self.products.append(product)
            }
        }
    }
    
    private class func buildProductDataRequest(_ categoryData:Dictionary<String,AnyObject>, page:Int=0) -> URL {
                
        let orderBy = "StyleSequence,UniqueId&$count=true"
        var select = "UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount"
        select += "," + (categoryData["select"] as! String)
        
        var filter = "(IsDropped eq false) and (ColorCount gt 0) and (ProductGroupPermanentName eq '\(DataSource.productGroup)') and (ProductGroupShowOnVizTool eq true) and (HasMainImage eq true)"
        filter += " and " + (categoryData["productsQuery"] as! String)
        
        var urlString = "\(DataSource.webSource)/\(categoryData["source"]!)?$top=\(DataSource.pageSize)&$skip=\(page * DataSource.pageSize)"
        
        urlString += "&$orderby=\(DataSource.encodeUrl(orderBy))"
        urlString += "&$select=\(DataSource.encodeUrl(select))"
        urlString += "&$filter=\(DataSource.encodeUrl(filter))"
        
        return URL(string: urlString)!
    }
}
