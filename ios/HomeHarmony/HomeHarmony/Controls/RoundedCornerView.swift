//
//  RoundedCornerView.swift
//  HomeHarmony
//
//  Created by Jaremy Longley on 3/15/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation

class RoundedCornerView: UIView {
    override func layoutSubviews() {
        super.layoutSubviews()
        let roundLayer = CAShapeLayer()
        roundLayer.path = self.roundTopCorners(radius: 20).path
        self.layer.mask = roundLayer
    }
}

extension UIView {
    
    func roundTopCorners(radius: CGFloat) -> CAShapeLayer {
        let path = UIBezierPath(roundedRect: bounds, byRoundingCorners: [.topLeft, .topRight], cornerRadii: CGSize(width: radius, height: radius))
        let mask = CAShapeLayer()
        mask.path = path.cgPath
        self.layer.mask = mask
        return mask
    }
    
}
