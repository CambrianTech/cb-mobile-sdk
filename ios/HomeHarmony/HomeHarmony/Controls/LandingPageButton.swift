//
//  LandingPageButton.swift
//  Prestige
//
//  Created by Joel Teply on 7/21/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

@IBDesignable
class LandingPageButton : UIButton {
    
    @IBInspectable var imageViewSize:CGSize = CGSize(width: 40, height: 40)
    
    var chevronView = UIImageView()
    
    @IBInspectable var chevronSize:CGSize = CGSize(width: 24, height: 24)
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        //self.backgroundColor = prestigeBGColor
        
        if let imageView = self.imageView, let titleLabel = self.titleLabel {
            imageView.contentMode = .scaleAspectFit
            imageView.frame = CGRect(x: self.frame.width * 0.1,
                                         y: (self.frame.height - imageViewSize.width)/2.0,
                                         width: self.imageViewSize.width,
                                         height: self.imageViewSize.width)
            
            if chevronView.superview == nil {
                self.chevronView.image = rightChevronImage()
                self.chevronView.contentMode = .scaleAspectFit
                self.addSubview(self.chevronView)
            }
            
            self.chevronView.tintColor = self.tintColor
            self.chevronView.frame = CGRect(x: self.frame.width - chevronSize.width - self.contentEdgeInsets.right,
                                            y: (self.frame.height - chevronSize.width)/2.0,
                                            width: chevronSize.width,
                                            height: chevronSize.height)
            
            
            let labelStartX = imageView.frame.maxX + self.frame.width * 0.04
            titleLabel.frame = CGRect(x: labelStartX, y: 0, width: self.frame.width - labelStartX, height: self.frame.size.height)
            
            self.layer.cornerRadius = 5
            
            self.clipsToBounds = true
        }
    }
}
