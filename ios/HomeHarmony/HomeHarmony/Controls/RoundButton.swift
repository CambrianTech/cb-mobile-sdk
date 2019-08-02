//
//  RoundButton.swift
//  Prestige
//
//  Created by joseph on 3/13/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation

@IBDesignable
class RoundButton : UIButton {
    
    override func layoutSubviews() {
        super.layoutSubviews()
        self.layer.cornerRadius = self.frame.height / 2
    }
}
