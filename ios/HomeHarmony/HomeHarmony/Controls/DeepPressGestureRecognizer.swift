//
//  DeepPressGestureRecognizer.swift
//  Prestige
//
//  Created by Joel Teply on 7/14/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import AudioToolbox
import UIKit.UIGestureRecognizerSubclass

class DeepPressGestureRecognizer: UIGestureRecognizer
{
    var vibrateOnDeepPress = true
    var threshold:CGFloat = 0.75
    var hardTriggerMinTime:TimeInterval = 1.0
    
    fileprivate var deepPressed: Bool = false
    fileprivate var deepPressedAt: TimeInterval = 0
    fileprivate var k_PeakSoundID:UInt32 = 1519
    fileprivate var hardAction:Selector?
    fileprivate var target: AnyObject?
    
    var firstRecordedThreshold:CGFloat = 0.0
    
    required init(target: AnyObject?, action: Selector, hardAction:Selector?=nil, threshold: CGFloat = 0.75)
    {
        self.target = target
        self.hardAction = hardAction
        self.threshold = threshold
        
        super.init(target: target, action: action)
    }
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent)
    {
        if let touch = touches.first
        {
            handleTouch(touch)
        }
    }
    
    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent)
    {
        if let touch = touches.first
        {
            handleTouch(touch)
        }
    }
    
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent)
    {
        super.touchesEnded(touches, with: event)
        
        state = deepPressed ? UIGestureRecognizerState.ended : UIGestureRecognizerState.failed
        
        deepPressed = false
    }
    
    fileprivate func handleTouch(_ touch: UITouch)
    {
        guard let _ = view , touch.force != 0 && touch.maximumPossibleForce != 0 else
        {
            return
        }
        
        let forcePercentage = (touch.force / touch.maximumPossibleForce)
        let currentTime = Date.timeIntervalSinceReferenceDate
        
        if !deepPressed && forcePercentage >= threshold
        {
            state = UIGestureRecognizerState.began
            firstRecordedThreshold = forcePercentage
            
            if vibrateOnDeepPress
            {
                AudioServicesPlaySystemSound(k_PeakSoundID)
            }
            
            deepPressedAt = Date.timeIntervalSinceReferenceDate
            deepPressed = true
        }
        else if deepPressed && forcePercentage <= 0
        {
            endGesture()
        }
        else if deepPressed && currentTime - deepPressedAt > hardTriggerMinTime && forcePercentage == 1.0
        {
            endGesture()
            
            if vibrateOnDeepPress
            {
                AudioServicesPlaySystemSound(k_PeakSoundID)
            }
            
            //fire hard press
            if let hardAction = self.hardAction, let target = self.target {
                _ = target.perform(hardAction, with: self)
            }
        }
    }
    
    func endGesture() {
        firstRecordedThreshold = 0
        state = UIGestureRecognizerState.ended
        deepPressed = false
    }
}

// MARK: DeepPressable protocol extension
protocol DeepPressable
{
    var gestureRecognizers: [UIGestureRecognizer]? {get set}
    
    func addGestureRecognizer(_ gestureRecognizer: UIGestureRecognizer)
    func removeGestureRecognizer(_ gestureRecognizer: UIGestureRecognizer)
    
    func setDeepPressAction(_ target: AnyObject, action: Selector)
    func removeDeepPressAction()
}

extension DeepPressable
{
    func setDeepPressAction(_ target: AnyObject, action: Selector)
    {
        let deepPressGestureRecognizer = DeepPressGestureRecognizer(target: target, action: action, threshold: 0.75)
        
        self.addGestureRecognizer(deepPressGestureRecognizer)
    }
    
    func removeDeepPressAction()
    {
        guard let gestureRecognizers = gestureRecognizers else
        {
            return
        }
        
        for recogniser in gestureRecognizers where recogniser is DeepPressGestureRecognizer
        {
            removeGestureRecognizer(recogniser)
        }
    }
}
