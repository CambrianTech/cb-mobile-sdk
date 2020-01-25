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
        
    @objc dynamic var id: String = ""
    override class func primaryKey() -> String? { return "id"}
    
    convenience init(key: String) {
        self.init()
        self.id = key
    }
    
    @objc dynamic var parentCategory: BrandCategory? = nil
    @objc dynamic var name = ""
    @objc dynamic var storeID = ""
    @objc dynamic var orderIndex = 0
    @objc dynamic var storeLink = ""
    
    //material, model specific
    @objc dynamic var assetPath = ""
    @objc dynamic var scale: Float = 1.0
    @objc dynamic var reflectivity: Float = 5.0
    
    //paint specific
    @objc dynamic var red = 0
    @objc dynamic var green = 0
    @objc dynamic var blue = 0
    
    @objc dynamic var info: String?
    
    @objc dynamic var opacity = 255
    
    override var color:UIColor {
           get {
               return UIColor(red: CGFloat(self.red) / 255.0,
                              green: CGFloat(self.green) / 255.0,
                              blue: CGFloat(self.blue) / 255.0,
                              alpha: CGFloat(self.opacity) / 255.0)
           }
           set {
               var fRed : CGFloat = 0
               var fGreen : CGFloat = 0
               var fBlue : CGFloat = 0
               var fAlpha: CGFloat = 0
               if !newValue.getRed(&fRed, green: &fGreen, blue: &fBlue, alpha: &fAlpha) {
                   return
               }
               
               self.red = Int(fRed * 255.0)
               self.green = Int(fGreen * 255.0)
               self.blue = Int(fBlue * 255.0)
               self.opacity = Int(fAlpha * 255.0)
           }
       }
}
