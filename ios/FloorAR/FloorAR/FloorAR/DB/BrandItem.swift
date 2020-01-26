//
//  BrandMaterial.swift
//
//
//  Created by Joel Teply on 6/7/16.
//
//

import Foundation
import RealmSwift

class BrandItem : SwatchObject {
        
    convenience init(key: String) {
        self.init()
        self.id = key
    }
    
    @objc dynamic var parentCategory: BrandCategory? = nil
    @objc dynamic var storeID = ""
    @objc dynamic var storeLink = ""
    
    //material, model specific
    @objc dynamic var scale: Float = 1.0
    @objc dynamic var reflectivity: Float = 5.0
    
    //paint specific
    @objc dynamic var info: String?
    @objc dynamic var opacity = 255
    
    override var color:UIColor {
           get {
               return UIColor(red: CGFloat(self.red) / 255.0,
                              green: CGFloat(self.green) / 255.0,
                              blue: CGFloat(self.blue) / 255.0,
                              alpha: CGFloat(self.opacity) / 255.0)
           }
    }
    
    override func getParent() -> HistoryItem? {
        return parentCategory
    }
}
