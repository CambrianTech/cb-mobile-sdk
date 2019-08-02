//
//  MessageBubble.swift
//  HomeHarmony
//
//  Created by Jaremy Longley on 3/20/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation

class GenericMessageBubble: UIView {
    var label: UILabel?
    
    override func draw(_ rect: CGRect) {
        //Draw a rounded message bubble with a tail
        let path = UIBezierPath(roundedRect: CGRect(x: 0, y: 0, width: self.frame.width, height: self.frame.height - 10), cornerRadius: 20.0)
    }
    
    func setMessage(_ message: String) {
        self.label = UILabel()
        label?.text = message
        label?.textAlignment = .center
        label?.frame = CGRect(x: 0, y: 0, width: self.frame.width, height: self.frame.height - 10)
        label?.textColor = UIColor(red: 51, green: 51, blue: 51)
        label?.numberOfLines = 2
        label?.font = UIFont.systemFont(ofSize: 20.0, weight: .semibold)
        self.addSubview(label!)
        self.bringSubview(toFront: label!)
    }
}

class MessageLowerRightIndicator: GenericMessageBubble {
    override func draw(_ rect: CGRect) {
        //Draw a rounded message bubble with a tail
        let path = UIBezierPath(roundedRect: CGRect(x: 0, y: 0, width: self.frame.width, height: self.frame.height - 10), cornerRadius: 20.0)
        //The tail
        path.move(to: CGPoint(x: rect.maxX - 50.0, y: rect.maxY - 10.0))
        path.addLine(to: CGPoint(x: rect.maxX - 30.0, y: rect.maxY))
        path.addLine(to: CGPoint(x: rect.maxX - 30.0, y: rect.maxY - 10.0))
        UIColor.white.setFill()
        path.fill()
        path.close()
    }
}

class MessageCenterIndicator: GenericMessageBubble {
    override func draw(_ rect: CGRect) {
        //Draw a rounded message bubble with a tail
        let path = UIBezierPath(roundedRect: CGRect(x: 0, y: 0, width: self.frame.width, height: self.frame.height - 10), cornerRadius: 20.0)
        //The tail
        path.move(to: CGPoint(x: rect.midX - 15.0, y: rect.maxY - 10.0))
        path.addLine(to: CGPoint(x: rect.midX, y: rect.maxY))
        path.addLine(to: CGPoint(x: rect.midX + 15.0, y: rect.maxY - 10.0))
        UIColor.white.setFill()
        path.fill()
        path.close()
    }
}
