//
//  DataSource.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/12/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit

class DataSource {
    
    let jsonPath = "datasource.json"
    let webSource:String
    let productGroup = "shawfloors"
    
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
                self.topLevelCategories = parseCategories(categoriesJSON: categories)
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
    
    private func parseCategories(categoriesJSON:Array<AnyObject>) -> [ProductCategory] {
        var categories: [ProductCategory] = []
        for categoryJson in categoriesJSON {
            categories.append(parseCategory(categoryJSON: categoryJson))
        }
        return categories
    }
    
    private func parseCategory(categoryJSON:AnyObject) -> ProductCategory {
        guard let parsed = categoryJSON as? Dictionary<String, AnyObject> else {
            fatalError("cannot parse json category")
        }
        
        let category = ProductCategory()
        category.code = parsed["name"] as! String
        category.name = parsed["displayName"] as! String
        category.thumbnailPath = Bundle.main.url(forResource: parsed["thumbnailPath"] as? String, withExtension: nil)
        
        requestCategoryData(category.code)
        
        return category
    }
    
     let test = "https://shawfloors.com/api/odata/Laminates?$top=1000&$skip=0&$orderby=StyleSequence,UniqueId&$count=true&$select=UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount,CollectionDesc,ColorFamilyDesc,CollectionDesc&$filter=(IsDropped eq false) and (ColorCount gt 0) and (ProductGroupPermanentName eq %27shawfloors%27) and (ProductGroupShowOnVizTool eq true) and (HasMainImage eq true) and (StaticRoomFlag eq true or HasRenderImage eq true) and (IsDefaultStyleColor eq true)"
    
    
    let pageSize = 1000
    
    private func encodeUrl(_ string:String) -> String {
        return string.addingPercentEncoding(withAllowedCharacters:NSCharacterSet.urlQueryAllowed)!
    }
    
    private func requestCategoryData(_ categoryCode:String, page:Int=0) {
        
        let categoryData = jsonCategories[categoryCode]!
        
        let orderBy = "StyleSequence,UniqueId&$count=true"
        var select = "UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount"
        select += "," + categoryData["select"]!
        
        let filter = "(IsDropped eq false) and (ColorCount gt 0) and (ProductGroupPermanentName eq '\(self.productGroup)') and (ProductGroupShowOnVizTool eq true) and (HasMainImage eq true) and (StaticRoomFlag eq true or HasRenderImage eq true) and (IsDuplicate eq false) and (IsDefaultStyleColor eq true) and (ProductCode ne '22')"
        
        var urlString = "\(self.webSource)/\(categoryData["source"]!)?$top=\(pageSize)&$skip=\(page * pageSize)"
        
        urlString += "&$orderby=\(encodeUrl(orderBy))"
        urlString += "&$select=\(encodeUrl(select))"
        urlString += "&$filter=\(encodeUrl(filter))"
        
        print(urlString)
    }
}
