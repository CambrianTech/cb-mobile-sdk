//
//  ProductColor+Extended.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

extension ProductColor {
    
    var product:Product{
        get {
            return parents[0]
        }
    }
}

