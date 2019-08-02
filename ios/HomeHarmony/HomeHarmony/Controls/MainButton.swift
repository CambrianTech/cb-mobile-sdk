//
//  MainButton.swift
//  HarmonyApp
//
//  Created by Jaremy Longley on 4/21/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation

class MainButton: UIButton {
    
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        if let imageView = self.imageView, let titleLabel = self.titleLabel {
            imageView.contentMode = .scaleAspectFill
            imageView.frame = CGRect(x: 0,
                                     y: 0,
                                     width: self.frame.height * 0.75,
                                     height: self.frame.height * 0.75)
            
            
            imageView.layer.cornerRadius = imageView.frame.height / 2
            imageView.backgroundColor = UIColor.white
            let centered = (self.frame.width / 2 - imageView.frame.width / 2)
            imageView.frame.origin.x = centered
            titleLabel.frame = CGRect(x: 0,
                                      y: self.frame.height * 0.8,
                                      width: self.frame.width,
                                      height: self.frame.height * 0.2)
            
            titleLabel.textAlignment = NSTextAlignment.center
            self.clipsToBounds = true
        }
    }
    
}
