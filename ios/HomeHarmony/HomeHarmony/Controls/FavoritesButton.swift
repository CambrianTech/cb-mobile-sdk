//
//  FavoritesButton.swift
//  HarmonyApp
//
//  Created by joseph on 5/3/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation

class FavoritesButton: RoundButton {
    
    var item:BrandItem? {
        didSet {
            if let item = item {
                update(item)
            }
        }
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
    }
    
    func update(_ item:BrandItem, duration: TimeInterval = 0.2) {
        UIView.animate(withDuration:duration, animations: {
            self.isHidden = false
            self.setImage(UIImage(named: "ic_favorite"), for: .normal)
            self.imageView?.tintColor = UIColor.black
        })
    }
}
