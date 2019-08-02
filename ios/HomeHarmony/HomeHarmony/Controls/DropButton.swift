//
//  DropButton.swift
//  HomeHarmony
//
//  Created by Jaremy Longley on 3/20/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation

internal protocol DropdownButtonDelegate : NSObjectProtocol {
    func dropButtonPressed(_ sender: AnyObject)
}

class DropButton: UIView {
    
    weak internal var delegate: DropdownButtonDelegate?
    @IBOutlet var label: UILabel?
    convenience init() {
        self.init()
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        self.backgroundColor = UIColor.white
        self.layer.cornerRadius = 20
        self.layer.borderColor = cambrianBlue.cgColor
        self.layer.borderWidth = 0.5
        self.label?.textColor = cambrianBlue
    }
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        self.backgroundColor = cambrianBlue
        self.label?.textColor = UIColor.white
        self.delegate?.dropButtonPressed(self)
    }
    
    func update(_ name: String) {
        self.label?.text = name
        self.backgroundColor = UIColor.white
        self.label?.textColor = cambrianBlue
    }
}
