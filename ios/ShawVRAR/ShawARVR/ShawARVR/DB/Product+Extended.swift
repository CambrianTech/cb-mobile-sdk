//
//  Product+Extended.swift..swift
//  ShawARVR
//
//  Created by Joel Teply on 12/13/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//


extension Product {    
    var category:ProductCategory {
        get {
            return parents[0]
        }
    }

    func sync(_ completion: @escaping () -> Void) {
        if (self.colors.count > 0) {
            return
        }
        DataSource.current.loadProductColors(self) { (colors) in
            self.colors.append(objectsIn: colors)
            completion()
        }
    }
}
