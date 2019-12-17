//
//  RoundButton.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit

@IBDesignable
class RoundButton: UIButton {
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }
    
    var textLabel:UILabel = UILabel()
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        self.clipsToBounds = true
        self.layer.cornerRadius = 0.5 * self.bounds.size.width
        
        let padding = self.bounds.width / 6
        self.imageEdgeInsets = UIEdgeInsetsMake(padding, padding, padding, padding);
        
        if (self.superview != nil && self.textLabel.superview == nil) {
            if let existingLabel = self.titleLabel, let text = self.title(for: .normal)?.uppercased() {
                self.textLabel.text = text
                
                let font = existingLabel.font!
                existingLabel.isHidden = true
                
                self.textLabel.font = font
                self.textLabel.textAlignment = .center
                self.textLabel.numberOfLines = 0
                
                let fontAttributes = [NSAttributedStringKey.font: font]
                let size = text.size(withAttributes: fontAttributes)

                var height = size.height
                var width = size.width
                
                if UI_USER_INTERFACE_IDIOM() == .phone {
                    width = 1.5 * self.bounds.size.width
                    height *= 2
                }
               
                self.textLabel.frame = CGRect(x:self.frame.midX - width / 2.0,
                                              y:self.frame.maxY + padding,
                                              width: width, height:height)
                
                self.textLabel.textColor = self.backgroundColor
                
                self.superview?.addSubview(self.textLabel)
            }
        }
        
    }

    // Only override draw() if you perform custom drawing.
    // An empty implementation adversely affects performance during animation.
    override func draw(_ rect: CGRect) {
        
        
        
        super.draw(rect)
    }

}
