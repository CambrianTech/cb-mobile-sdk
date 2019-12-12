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
    
    let test = "https://shawfloors.com/api/odata/Hardwoods?$top=1000&$skip=0&$orderby=StyleSequence,UniqueId&$count=true&$select=UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount,CollectionDesc,ColorFamilyDesc,Shade,SurfaceTextureShortDesc,InstallationType,CollectionDesc&$filter=(IsDropped%20eq%20false)%20and%20(ColorCount%20gt%200)%20and%20(ProductGroupPermanentName%20eq%20%27shawfloors%27)%20and%20(ProductGroupShowOnVizTool%20eq%20true)%20and%20(HasMainImage%20eq%20true)%20and%20(StaticRoomFlag%20eq%20true%20or%20HasRenderImage%20eq%20true)%20and%20(IsDefaultStyleColor%20eq%20true)"
    
    var topLevelCategories: [ProductCategory] = []
    var jsonCategories:Array<AnyObject>
    
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
                self.jsonCategories = categories
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
        
        return category
    }
}
