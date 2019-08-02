//
//  AlphaTouchableView.swift
//  ShawDemo
//
//  Created by Joel Teply on 8/31/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

class AlphaTouchableView : UIView {
    override func point(inside point: CGPoint, with event: UIEvent?) -> Bool {
        for subview in subviews as [UIView] {
            if !subview.isHidden && subview.alpha > 0 && subview.isUserInteractionEnabled && subview.point(inside: convert(point, to: subview), with: event) {
                return true
            }
        }
        return false
    }

}
