//
//  StandardButton.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit

class StandardButton: UIButton {

    override func layoutSubviews() {
        super.layoutSubviews()
        
        self.backgroundColor = isSelected ? UIColor(red: 1.0, green: 1.0, blue: 1.0, alpha: 0.5) : UIColor(red: 0.0, green: 0.0, blue: 0.0, alpha: 1.0)
        
        self.setTitleColor(UIColor.white, for: .selected)
        self.setTitleColor(UIColor.black, for: .normal)
        
        self.layer.borderWidth = 1
        self.layer.borderColor = isSelected ? UIColor.white.cgColor : UIColor.black.cgColor
    }

    override var isSelected: Bool {
        didSet {
            layoutSubviews()
        }
    }
}
