//
//  BrandItem_Extended.swift
//  HomeHarmony
//
//  Created by Joel Teply on 11/7/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift

extension BrandItem {
        
    func needsDarkText() -> Bool {
        let calculation = ((self.red * 299) + (self.green * 587) + (self.blue * 114)) / 1000
        if calculation < 125 {
            return false
        } else {
            return true
        }
    }
    
    var text: String? {
        return self.name
    }
}



