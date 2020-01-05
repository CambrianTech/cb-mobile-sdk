//
//  ProductStyle.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

class ProductColor: CBDataObject {
    
    let parents = LinkingObjects(fromType: Product.self, property: "colors")
    let variations = List<ProductVariation>()
    
    private static var _shared = Product()
    static var shared:CBDataObject {
        get {
            return _shared
        }
    }
    
    func needsUpdate() -> Bool {
        if (Date().days(from: self.updated) > 3) {
            return true
        }
        return false
    }
    
    func getDataUrl() -> URL? {
        return nil
    }
    
    func parseObjects(data: Dictionary<String, AnyObject>, realm:Realm) {
        
    }
}
