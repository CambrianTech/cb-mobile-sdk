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
        
        self.backgroundColor = isSelected ? UIColor.gray : UIColor.clear
        
        self.setTitleColor(UIColor.white, for: .selected)
        self.setTitleColor(UIColor.black, for: .normal)
        
        self.layer.borderWidth = 1
        self.layer.borderColor = isSelected ? UIColor.gray.cgColor : UIColor.lightGray.cgColor
    }

    override var isSelected: Bool {
        didSet {
            layoutSubviews()
        }
    }
    
    
}
