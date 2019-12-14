//
//  ProductCategory+Extended.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

extension ProductCategory {
    var thumbnailUrl : URL? {
        return URL(string: thumbnailPath)
    }
    
}

