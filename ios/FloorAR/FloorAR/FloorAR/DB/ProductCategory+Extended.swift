//
//  ProductCategory+Extended.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

extension ProductCategory {
    
    convenience init (_ json:Dictionary<String, AnyObject>) {
        self.init()
        
        self.jsonString = json.jsonString
        self.code = json["name"] as! String
        self.name = json["displayName"] as! String
        self.thumbnailPath = Bundle.main.url(forResource: json["thumbnailPath"] as? String, withExtension: nil)!.absoluteString
    }
    
    var thumbnailUrl : URL? {
        return URL(string: thumbnailPath)
    }
    
    func sync(_ completion: @escaping () -> Void) {
        if (self.products.count > 0) {
            return
        }
        
        let categoryID = self.code
        DispatchQueue.global(qos: .background).async {
            Product.loadProducts(categoryID) { (products) in
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

