//
//  BrandCategory_Extended.swift
//  HomeHarmony
//
//  Created by Joel Teply on 11/7/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift

extension BrandCategory {
    
    func needsDarkText() -> Bool {
        if self.color == UIColor(red: 0, green: 0, blue: 0) {
            if let drawable = self.displayItem {
                return drawable.needsDarkText()
            }
        } else {
            // Calculation for darkness or lightness of color
            let calculation = ((self.red * 299) + (self.green * 587) + (self.blue * 114)) / 1000
            if calculation < 125 {
                return false
            } else {
                return true
            }
        }
        return false
    }
    
    var text: String? {
        return self.name
    }
}








