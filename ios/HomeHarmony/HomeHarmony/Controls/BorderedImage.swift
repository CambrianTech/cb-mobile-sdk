//
//  BorderButton.swift
//  Prestige
//
//  Created by Joel Teply on 7/25/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

@IBDesignable
class BorderedImage : UIImageView {
    
    @IBInspectable var borderRadius: CGFloat = 3.0 {
        didSet {
            setNeedsLayout()
        }
    }
    
    @IBInspectable var borderColor:UIColor = UIColor.white {
        didSet {
            self.layer.borderColor = borderColor.cgColor
        }
    }
    
    @IBInspectable var borderWidth:CGFloat = 1.0 {
        didSet {
            self.layer.borderWidth = borderWidth
        }
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        self.isOpaque = false
        self.layer.cornerRadius = self.borderRadius;
        self.layer.borderWidth = self.borderWidth;
        
        self.clipsToBounds = true
        //self.contentMode = .scaleAspectFit
    }
}
