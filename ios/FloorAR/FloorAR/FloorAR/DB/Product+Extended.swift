//
//  Product+Extended.swift..swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//


import Alamofire

extension Product {
    
    convenience init (_ json:Dictionary<String, AnyObject>) {
        self.init()
        
        self.jsonString = json.jsonString
        self.code = json["UniqueId"] as! String
        self.name = json["SellingStyleName"] as! String
        self.thumbnailPath = DataSource.getThumbnailPath(self.code)
    }
    
    var thumbnailUrl : URL? {
        return URL(string: thumbnailPath)
    }
    
    var category:ProductCategory {
        get {
            return parents[0]
        }
    }
    
    var jsonData:[String: AnyObject] {
        get {
            return self.jsonString.jsonData
        }
    }
    
    var styleNumber:String {
        get {
            return self.jsonData["SellingStyleNbr"] as! String
        }
    }
    
    class func first() -> Product? {
        guard let realmResults = DataController.sharedInstance.productContext?.objects(Product.self) else { return nil }
        return realmResults.first
    }
    
    class func random() -> Product? {
        guard let realmResults = DataController.sharedInstance.productContext?.objects(Product.self) else { return nil }
        let index = Int (arc4random_uniform(UInt32(realmResults.count)));
        return realmResults[index];
    }

    func sync(_ completion: @escaping () -> Void) {
        if (self.colors.count > 0) {
            completion()
        } else {
            let categoryID = self.category.code
            let styleNumber = self.styleNumber
            DispatchQueue.global(qos: .background).async {
                ProductColor.loadProductColors(categoryID, styleNumber) { (colors) in
                    DispatchQueue.main.async {
                        try! DataSource.current.realm.write {
                            self.colors.append(objectsIn: colors)
                            if let json = colors.first?.jsonString {
                                print(json)
                            }
                        }
                        completion()
                    }
                }
            }
        }
    }
    
    class func loadProducts(_ categoryCode:String, _ completion: @escaping ([Product]) -> Void) {
        
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

    private class func parseProducts(_ _productsJSON:Array<Dictionary<String, AnyObject>>) -> [Product] {
        var products: [Product] = []
        for productJson in _productsJSON {
            let product = Product(productJson)
            products.append(product)
        }
        return products
    }
    
    private class func buildProductDataRequest(_ categoryCode:String, page:Int=0) -> URL {
        
        let categoryData = DataSource.current.jsonCategories[categoryCode]!
        
        let orderBy = "StyleSequence,UniqueId&$count=true"
        var select = "UniqueId,SellingStyleNbr,SellingColorNbr,SellingStyleName,SellingColorName,StaticRoomFlag,Vignette,ColorCount,MSRPRange,HasSwatchImage,SampleCount"
        select += "," + (categoryData["select"] as! String)
        
        var filter = "(IsDropped eq false) and (ColorCount gt 0) and (ProductGroupPermanentName eq '\(DataSource.current.productGroup)') and (ProductGroupShowOnVizTool eq true) and (HasMainImage eq true)"
        filter += " and " + (categoryData["productsQuery"] as! String)
        
        var urlString = "\(DataSource.current.webSource)/\(categoryData["source"]!)?$top=\(DataSource.pageSize)&$skip=\(page * DataSource.pageSize)"
        
        urlString += "&$orderby=\(DataSource.encodeUrl(orderBy))"
        urlString += "&$select=\(DataSource.encodeUrl(select))"
        urlString += "&$filter=\(DataSource.encodeUrl(filter))"
        
        return URL(string: urlString)!
    }
    
    var ppi:Int {
        get {
            return DataSource.current.getImagePPI(self.code)
        }
    }
    
    var dpcm:Float {
        get {
            return Float(self.ppi) / 2.54
        }
    }
    
    var jsonCommand:String {
        var data = Dictionary<String, AnyObject>()
        data["name"] = self.name as NSString
        data["dpcm"] = NSNumber(value:self.dpcm)
        return data.jsonString
    }
}
