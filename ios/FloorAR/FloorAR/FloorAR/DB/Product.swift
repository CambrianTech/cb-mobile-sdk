//
//  Product.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

class Product : CBDataObject {
    
    @objc dynamic var width:Int = 1024
    @objc dynamic var height:Int = 1024
    
    let parents = LinkingObjects(fromType: ProductCategory.self, property: "products")
    let installationMethods = List<InstallationMethod>()
    let colors = List<ProductColor>()
    
    private static var _shared = Product()
    static var shared:CBDataObject {
        get {
            return _shared
        }
    }
    
    func needsUpdate() -> Bool {
        return self.colors.count == 0
    }
    
    func getDataUrl() -> URL? {
        return Product.buildProductColorsDataRequest(self.category.jsonString.jsonData, self.styleNumber)
    }
    
    func parseObjects(data: Dictionary<String, AnyObject>) {
        guard let colorsJSON = data["value"] as? Array<Dictionary<String, AnyObject>> else {
            fatalError("no value attribute in json data")
        }
        
        for colorJson in colorsJSON {
            let color = ProductColor(colorJson)
            try! DataSource.current.realm.write {
                self.colors.append(color)
            }
        }
    }
    
    private class func buildProductColorsDataRequest( _ categoryData:Dictionary<String,AnyObject>, _ styleNumber:String, page:Int=0) -> URL {
        
        let orderBy = "StyleSequence,UniqueId&$count=true"
        var select = "UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount"
        select += "," + (categoryData["select"] as! String)
        
        var filter = "(IsDropped eq false) and (ColorCount gt 0) and (ProductGroupPermanentName eq '\(DataSource.productGroup)') and (ProductGroupShowOnVizTool eq true) and (HasMainImage eq true)"
        filter += " and " + (categoryData["colorsQuery"] as! String)
        filter += " and (SellingStyleNbr eq '\(styleNumber)')"
        
        var urlString = "\(DataSource.webSource)/\(categoryData["source"]!)?$top=\(DataSource.pageSize)&$skip=\(page * DataSource.pageSize)"
        
        urlString += "&$orderby=\(DataSource.encodeUrl(orderBy))"
        urlString += "&$select=\(DataSource.encodeUrl(select))"
        urlString += "&$filter=\(DataSource.encodeUrl(filter))"
        
        //print(urlString)
        
        return URL(string: urlString)!
    }
}
