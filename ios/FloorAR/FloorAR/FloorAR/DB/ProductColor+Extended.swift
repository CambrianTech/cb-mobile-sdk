//
//  ProductColor+Extended.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import Alamofire

extension ProductColor {
    
    convenience init (_ json:Dictionary<String, AnyObject>) {
        self.init()
        
        self.jsonString = json.jsonString
        self.code = json["UniqueId"] as! String
        self.name = json["SellingColorName"] as! String
        self.thumbnailPath = DataSource.getThumbnailPath(self.code)
    }
    
    var thumbnailUrl : URL? {
        return URL(string: thumbnailPath)
    }
    
    var product:Product{
        get {
            return parents[0]
        }
    }
    
    class func loadProductColors(_ categoryID:String, _ styleNumber:String, _ completion: @escaping ([ProductColor]) -> Void) {
        
        //create the url with NSURL
        let url = buildProductColorsDataRequest(categoryID, styleNumber)

        AF.request(url).responseJSON { response in
            if let json = response.value as? Dictionary<String, AnyObject>,
                //let count = json["@odata.count"] as? Int,
                let categoriesJSON = json["value"] as? Array<Dictionary<String, AnyObject>> {

                let productColors = parseProductColors(categoriesJSON)
                completion(productColors)
            }
        }
    }
    
    private class func buildProductColorsDataRequest( _ categoryID:String, _ styleNumber:String, page:Int=0) -> URL {
        guard let categoryData = DataSource.current.jsonCategories[categoryID] else {
            fatalError("cannot get json category")
        }
        
        let orderBy = "StyleSequence,UniqueId&$count=true"
        var select = "UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount"
        select += "," + (categoryData["select"] as! String)
        
        var filter = "(IsDropped eq false) and (ColorCount gt 0) and (ProductGroupPermanentName eq '\(DataSource.current.productGroup)') and (ProductGroupShowOnVizTool eq true) and (HasMainImage eq true)"
        filter += " and " + (categoryData["colorsQuery"] as! String)
        filter += " and (SellingStyleNbr eq '\(styleNumber)')"
        
        var urlString = "\(DataSource.current.webSource)/\(categoryData["source"]!)?$top=\(DataSource.pageSize)&$skip=\(page * DataSource.pageSize)"
        
        urlString += "&$orderby=\(DataSource.encodeUrl(orderBy))"
        urlString += "&$select=\(DataSource.encodeUrl(select))"
        urlString += "&$filter=\(DataSource.encodeUrl(filter))"
        
        //print(urlString)
        
        return URL(string: urlString)!
    }
    
    private class func parseProductColors(_ _productsJSON:Array<Dictionary<String, AnyObject>>) -> [ProductColor] {
        var colors: [ProductColor] = []
        for productJson in _productsJSON {
            let color = ProductColor(productJson)
            colors.append(color)
        }
        return colors
    }
    
    public var defaultVariation: ProductVariation {
        get {
            let variation = ProductVariation(self)
            variation.code = self.code
            variation.name = self.name
            
            return variation
        }
    }
    
    var jsonCommand:String {
        let variations = self.variations.count > 0 ? Array(self.variations) : [self.defaultVariation]
        return "{\"name\":\"\(name)\", \"variations\":[\(variations.compactMap({$0.jsonCommand}).joined(separator:","))]}"
    }
} 

