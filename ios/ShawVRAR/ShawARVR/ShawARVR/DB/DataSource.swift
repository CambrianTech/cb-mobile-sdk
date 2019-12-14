//
//  DataSource.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/12/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import Alamofire
import RealmSwift

class DataSource {
    
    let jsonPath = "datasource.json"
    let webSource:String
    let productGroup = "shawfloors"
    let baseImagePath = "https://shawfloors.scene7.com/is/image";
    let imageSize = 320
    
    let realm = try! Realm()
    
    var topLevelCategories: [ProductCategory] = []
    var jsonCategories:Dictionary<String, Dictionary<String, String>> = Dictionary<String, Dictionary<String, String>>()
    
    private init() {
        if let path = Bundle.main.path(forResource: jsonPath, ofType: nil) {
            do {
                let data = try Data(contentsOf: URL(fileURLWithPath: path), options: .mappedIfSafe)
                let jsonResult = try JSONSerialization.jsonObject(with: data, options: .mutableLeaves)
                
                guard let parsed = jsonResult as? Dictionary<String, AnyObject> else {
                    fatalError("cannot parse json data")
                }
                
                guard let source = parsed["source"] as? String, let categories = parsed["categories"] as? Array<AnyObject> else {
                    fatalError("no source attribute in json data")
                }
                
                self.webSource = source
                for categoryJson in categories {
                    let category = categoryJson as! Dictionary<String, String>
                    jsonCategories[category["name"]!] = category
                }
                self.topLevelCategories = parseProductCategories(categories)
                return
            }
            catch {
                fatalError("required datasource caused error")
            }
        }
        fatalError("cannot find required datasource")
    }
    
    private static var _client: DataSource?
    
    static var current: DataSource {
        get {
            if let datasource = DataSource._client {
                return datasource
            }
            let datasource = DataSource()
            DataSource._client = datasource
            return datasource
        }
    }
    
    private func parseProductCategories(_ categoriesJSON:Array<AnyObject>) -> [ProductCategory] {
        var categories: [ProductCategory] = []
        for categoryJson in categoriesJSON {
            let category = parseProductCategory(categoryJson)
            categories.append(category)
            let categoryCode = category.code
            
            try! realm.write {
                realm.add(category)
            }
            
            DispatchQueue.global(qos: .background).async {
                self.loadProducts(categoryCode, { products in
                    DispatchQueue.main.async {
                        for product in products {
                            try! self.realm.write {
                                category.products.append(product)
                            }
                        }
                    }
                })
            }
        }
        return categories
    }
    
    private func parseProductCategory(_ categoryJSON:AnyObject) -> ProductCategory {
        guard let parsed = categoryJSON as? Dictionary<String, AnyObject> else {
            fatalError("cannot parse json category")
        }
        
        let category = ProductCategory()
        category.code = parsed["name"] as! String
        category.name = parsed["displayName"] as! String
        category.thumbnailPath = Bundle.main.url(forResource: parsed["thumbnailPath"] as? String, withExtension: nil)!.absoluteString
        
        return category
    }
    
    private func parseProducts(_ _productsJSON:Array<Dictionary<String, AnyObject>>) -> [Product] {
        var products: [Product] = []
        for productJson in _productsJSON {
            let product = parseProduct(productJson)
            products.append(product)
        }
        return products
    }
    
    private func parseProduct(_ categoryJSON:Dictionary<String, AnyObject>) -> Product {
        
        let product = Product()
        
        product.code = categoryJSON["UniqueId"] as! String
        product.name = categoryJSON["SellingStyleName"] as! String
        product.thumbnailPath = "\(baseImagePath)/ShawIndustries/\(product.code)_MAIN?fit=crop&wid=\(imageSize)&hei=\(imageSize)&fmt=jpg"
        return product
    }
    
    let pageSize = 1000
    
    private func encodeUrl(_ string:String) -> String {
        return string.addingPercentEncoding(withAllowedCharacters:NSCharacterSet.urlQueryAllowed)!
    }
    
    func loadProducts(_ categoryCode:String, _ completion: @escaping ([Product]) -> Void) {
        
        //create the url with NSURL
        let url = buildProductDataRequest(categoryCode)

        AF.request(url).responseJSON { response in
            if let json = response.value as? Dictionary<String, AnyObject>,
                //let count = json["@odata.count"] as? Int,
                let productsJSON = json["value"] as? Array<Dictionary<String, AnyObject>> {
                
                let products = self.parseProducts(productsJSON)
                completion(products)
            }
        }
    }
    
    private func buildProductDataRequest(_ categoryCode:String, page:Int=0) -> URL {
        
        let categoryData = jsonCategories[categoryCode]!
        
        let orderBy = "StyleSequence,UniqueId&$count=true"
        var select = "UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount"
        select += "," + categoryData["select"]!
        
        var filter = "(IsDropped eq false) and (ColorCount gt 0) and (ProductGroupPermanentName eq '\(self.productGroup)') and (ProductGroupShowOnVizTool eq true) and (HasMainImage eq true)"
        filter += " and " + categoryData["productsQuery"]!
        
        var urlString = "\(self.webSource)/\(categoryData["source"]!)?$top=\(pageSize)&$skip=\(page * pageSize)"
        
        urlString += "&$orderby=\(encodeUrl(orderBy))"
        urlString += "&$select=\(encodeUrl(select))"
        urlString += "&$filter=\(encodeUrl(filter))"
        
        return URL(string: urlString)!
    }
    
    func loadProductColors(_ product:Product, _ completion: @escaping ([ProductColor]) -> Void) {
        
        //create the url with NSURL
        let url = buildProductColorsDataRequest(product)

        AF.request(url).responseJSON { response in
            if let json = response.value as? Dictionary<String, AnyObject>,
                //let count = json["@odata.count"] as? Int,
                let categoriesJSON = json["value"] as? Array<Dictionary<String, AnyObject>> {

                let productColors = self.parseProductColors(categoriesJSON)
                completion(productColors)
            }
        }
    }
    
    private func buildProductColorsDataRequest( _ product:Product, page:Int=0) -> URL {
        guard let categoryData = jsonCategories[product.category.code] else {
            fatalError("cannot get json category")
        }
        
        let orderBy = "StyleSequence,UniqueId&$count=true"
        var select = "UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount"
        select += "," + categoryData["select"]!
        
        var filter = "(IsDropped eq false) and (ColorCount gt 0) and (ProductGroupPermanentName eq '\(self.productGroup)') and (ProductGroupShowOnVizTool eq true) and (HasMainImage eq true)"
        filter += " and " + categoryData["colorsQuery"]!
        
        var urlString = "\(self.webSource)/\(categoryData["source"]!)?$top=\(pageSize)&$skip=\(page * pageSize)"
        
        urlString += "&$orderby=\(encodeUrl(orderBy))"
        urlString += "&$select=\(encodeUrl(select))"
        urlString += "&$filter=\(encodeUrl(filter))"
        
        return URL(string: urlString)!
    }
    
    private func parseProductColors(_ _productsJSON:Array<Dictionary<String, AnyObject>>) -> [ProductColor] {
        var colors: [ProductColor] = []
        for productJson in _productsJSON {
            colors.append(parseProductColor(productJson))
        }
        return colors
    }
    
    private func parseProductColor(_ categoryJSON:Dictionary<String, AnyObject>) -> ProductColor {
        
        let color = ProductColor()
        
        color.code = categoryJSON["UniqueId"] as! String
        color.name = categoryJSON["SellingStyleName"] as! String
        //color.thumbnailPath = "\(baseImagePath)/ShawIndustries/\(color.code)_MAIN?fit=crop&wid=\(imageSize)&hei=\(imageSize)&fmt=jpg"
        
        return color
    }
}
