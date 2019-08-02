//
//  ColorPickCircle.swift
//  HarmonyApp
//
//  Created by joseph on 4/10/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation


class ColorPickCircle : UIView {
    
    let colorLayer = CAShapeLayer()
    
    init() {
        let frame = CGRect(x: 0, y: 0, width: 50, height: 50)
        super.init(frame: frame)
        self.isHidden = true
        self.alpha = 0
        addLayer()
        
        let screen = UIScreen.main.bounds
        
        self.center.x = CGFloat.random(min: screen.width * 0.1, max: screen.width * 0.9)
        self.center.y = CGFloat.random(min: screen.height * 0.1, max: screen.width * 0.9)
    }
    
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        self.layer.cornerRadius = self.frame.width / 2
    }
    
    func addLayer() {
        let path = UIBezierPath(arcCenter: CGPoint(x: frame.width/2, y: frame.height/2),
                                radius: (frame.width / 2) - 4,
                                startAngle: 0,
                                endAngle: CGFloat(2 * Float.pi),
                                clockwise: true)
        
        colorLayer.path = path.cgPath
        colorLayer.cornerRadius = colorLayer.frame.width / 2
        colorLayer.fillColor = UIColor.gray.cgColor
        
        self.layer.addSublayer(colorLayer)
        self.layer.backgroundColor = UIColor.black.withAlphaComponent(0.5).cgColor
    }
    
    func setColor(_ color: UIColor) {
        self.colorLayer.fillColor = color.cgColor
        if(color.isLight) {
            self.layer.backgroundColor = UIColor.black.withAlphaComponent(0.5).cgColor
        } else {
            self.layer.backgroundColor = UIColor.white.withAlphaComponent(0.5).cgColor
        }
    }
    
    
    func update(color: UIColor, position: CGPoint, offset: CGPoint) {
        self.isHidden = false
        
        UIView.animate(withDuration: 0.5, animations: {
            self.alpha = 1
            self.center.x = position.x + offset.x
            self.center.y = position.y + offset.y
            self.setColor(color)
        })
    }
    
    func hide() {
        UIView.animate(withDuration: 0.3, animations: {
            self.alpha = 0
        }, completion: { (completed) -> Void in
            self.isHidden = true
        })
    }
}
