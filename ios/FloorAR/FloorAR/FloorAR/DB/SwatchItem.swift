//
//  SwatchItem.swift
//  DesignUp
//
//  Created by Joel Teply on 1/25/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

protocol SwatchItem : NSObjectProtocol {
    var color:UIColor {get}
    var thumbnailUrl:URL? {get}
}

class SwatchObject : Object, SwatchItem {
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    @objc dynamic var assetPath:String = ""
    @objc dynamic var name:String = ""
    @objc dynamic var orderIndex = 0
    
    @objc dynamic var red = 0
    @objc dynamic var green = 0
    @objc dynamic var blue = 0
    
    var color:UIColor {
        get {
            return UIColor(red: self.red / 255, green: self.green / 255, blue: self.blue / 255)
        }
    }
    
    var thumbnailUrl : URL? {
        if self.assetPath.count > 0 {
            let pathString = "bundle/\(self.assetPath)/Base_Color.png"
            return Bundle.main.url(forResource:pathString, withExtension: nil)
        }
        return nil
    }
    
    func needsDarkText() -> Bool {
        let calculation = ((self.red * 299) + (self.green * 587) + (self.blue * 114)) / 1000
        if calculation < 125 {
            return false
        } else {
            return true
        }
    }
}
