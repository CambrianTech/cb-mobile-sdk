//
//  BrandDrawable.swift
//  HomeHarmony
//
//  Created by Joel Teply on 11/8/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

protocol Drawable : NSObjectProtocol {
    
    func needsDarkText() -> Bool
    
    var text: String? {get}
    
    var view: UIView? {get}
}
