//
//  AddCollectionView.swift
//  Prestige
//
//  Created by Joel Teply on 7/14/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

//http://stackoverflow.com/questions/16189682/delete-cell-from-uicollectionview/16190291#16190291

internal protocol DetailsCollectionViewDelegate : UICollectionViewDelegate {
    func addCollectionViewItem()
    func longPressCollectionViewItem(_ indexPath:IndexPath)
    //func previewForCollectionViewItem(_ indexPath:IndexPath) -> UIView?
    //func detailsForCollectionViewItem(_ indexPath:IndexPath)
}


@IBDesignable
class DetailsCollectionView : UICollectionView, UIGestureRecognizerDelegate {
    
    @IBInspectable
    var canAddItems: Bool = false {
        didSet {
            self.addButton.isHidden = !canAddItems
        }
    }
    
    @IBInspectable
    var addColor:UIColor = UIColor.blue {
        didSet {
            self.addButton.backgroundColor = addColor
        }
    }

    @IBInspectable
    var addBorderColor:UIColor = UIColor.white {
        didSet {
            self.addButton.borderColor = addBorderColor
        }
    }
    
    @IBInspectable
    var addTextColor:UIColor = UIColor.white {
        didSet {
            self.addButton.textColor = addTextColor
        }
    }
    
    @IBInspectable
    var addBorderWidth: CGFloat = 1.0 {
        didSet {
            self.addButton.borderWidth = addBorderWidth
        }
    }
    
    @IBInspectable 
    var addRadius: CGFloat = 50.0 {
        didSet {
            setNeedsLayout()
        }
    }
    
    @IBInspectable
    var chevronSize:CGSize = CGSize(width: 40, height: 60)

    fileprivate var initialized = false
    
    var addButton:DetailsAddButton = DetailsAddButton(frame: CGRect(x: 0,y: 0,width: 50,height: 50))
    fileprivate var editDelegate:DetailsCollectionViewDelegate?
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        if let superview = self.superview , !initialized {
            let longPressGesture = UILongPressGestureRecognizer(target: self, action: #selector(self.longPress))
            longPressGesture.minimumPressDuration = 1.0
            longPressGesture.delegate = self
            self.addGestureRecognizer(longPressGesture)
            
            //let deepPressGesture = DeepPressGestureRecognizer(target: self, action: #selector(self.deepPress), hardAction: #selector(self.deepPressHard))
            //deepPressGesture.delegate = self
            //self.addGestureRecognizer(deepPressGesture)
            
            self.addButton.addTarget(self, action: #selector(self.addButtonPressed), for: UIControlEvents.touchUpInside)
            superview.addSubview(self.addButton)
            self.addButton.isHidden = !canAddItems

            initialized = true
        }
        
        
        self.addButton.frame = CGRect(x: self.frame.size.width - 2.5 * addRadius,
                                      y: self.frame.size.height - 2.5 * addRadius,
                                      width: 2 * addRadius,
                                      height: 2 * addRadius)
        
    }
    
    override func reloadData() {
        super.reloadData()
    }
    
    func leftmostIndexPath() -> IndexPath? {
        //TODO: use spacing/margin or item size instead of hardcoded 20
        let visibleRect = CGRect(origin: self.contentOffset, size: self.bounds.size)
        let visiblePoint = CGPoint(x: 20 + visibleRect.minX, y: visibleRect.midY)
        return self.indexPathForItem(at: visiblePoint)
    }
    
    func rightmostIndexPath() -> IndexPath? {
        //TODO: use spacing/margin or item size instead of hardcoded 20
        let visibleRect = CGRect(origin: self.contentOffset, size: self.bounds.size)
        let visiblePoint = CGPoint(x: visibleRect.maxX - 20, y: visibleRect.midY)
        return self.indexPathForItem(at: visiblePoint)
    }
    
    @objc func addButtonPressed() {
        if let delegate = delegate as? DetailsCollectionViewDelegate {
            delegate.addCollectionViewItem()
        }
    }
    
    @objc func longPress(_ gesture:UILongPressGestureRecognizer) {
        if gesture.state == UIGestureRecognizerState.began {
            let point = gesture.location(in: self)
            let indexPath = self.indexPathForItem(at: point)
            if let indexPath = indexPath, let delegate = delegate as? DetailsCollectionViewDelegate {
                delegate.longPressCollectionViewItem(indexPath)
            }
        }
        
    }
    
    /*
    func deepPress(_ gesture:UILongPressGestureRecognizer) {
        if gesture.state == .began {
            let point = gesture.location(in: self)
            let indexPath = self.indexPathForItem(at: point)
            if let indexPath = indexPath, let delegate = delegate as? DetailsCollectionViewDelegate {
                if let view = delegate.previewForCollectionViewItem(indexPath) {
                    displayView(view, fromPoint: point, indexPath: indexPath)
                }
            }
        } else if !animatingDeepPress && (gesture.state == .ended || gesture.state == .cancelled || gesture.state == .failed) {
            destroyOverlays()
        }
    }
    
    var animatingDeepPress = false
    func deepPressHard(_ gesture:UILongPressGestureRecognizer) {

        if let view = self.displayedView, let keyWindow = UIApplication.shared.keyWindow {
            animatingDeepPress = true
            let endFrame = keyWindow.frame
            UIView.animate(withDuration: 0.1, animations: {
                view.frame = endFrame
                }, completion: { (completed) -> Void in
                    self.animatingDeepPress = false
                    self.destroyOverlays()
                    if let delegate = self.delegate as? DetailsCollectionViewDelegate, let indexPath = self.previewedIndexPath {
                        delegate.detailsForCollectionViewItem(indexPath)
                    }
                }
            )
        }
 
    }
 */
    
    func destroyOverlays() {
        if let view = displayedView {
            view.removeFromSuperview()
            displayedView = nil
        }
        if let view = blurredView {
            view.removeFromSuperview()
            blurredView = nil
        }
    }
    
    fileprivate var blurredView:UIVisualEffectView? = nil
    fileprivate var displayedView:UIView? = nil
    fileprivate var previewedIndexPath:IndexPath?
    
    func displayView(_ view : UIView, fromPoint:CGPoint, indexPath:IndexPath) {
        
        if let keyWindow = UIApplication.shared.keyWindow {
            
            let blurEffect = UIBlurEffect(style: .dark)
            let blurView = UIVisualEffectView(effect: blurEffect)
            blurView.frame = keyWindow.bounds
            
            keyWindow.addSubview(blurView)
            blurredView = blurView

            view.clipsToBounds = true
            view.frame = CGRect(x: 0, y: 0, width: 100, height: 100)
            view.frame.origin = fromPoint
            view.bounds = view.frame
        
            keyWindow.addSubview(view)
            
            let yBorder:CGFloat = 50
            let xBorder:CGFloat = 30
            let endFrame = CGRect(x: xBorder, y: yBorder,
                                  width: keyWindow.frame.size.width - 2.0 * xBorder,
                                  height: keyWindow.frame.size.height - 2.0 * yBorder)
            
            UIView.animate(withDuration: 0.2, animations: {
                view.frame = endFrame
                view.bounds = endFrame

                }, completion: { (Bool) -> Void in
                    //self.previewedIndexPath = indexPath
                }
            )
            
            displayedView = view
        }
    }
}

class DetailsAddButton: UIButton {
    
    var borderWidth: CGFloat = 1.0
    var borderColor = UIColor.white
    var textColor = UIColor.white
    
    fileprivate var initialized = false
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        
        if (!initialized) {
            self.contentMode = UIViewContentMode.scaleAspectFill
            self.imageView?.contentMode = UIViewContentMode.scaleAspectFill;
            self.contentHorizontalAlignment = UIControlContentHorizontalAlignment.center;
            self.contentVerticalAlignment = UIControlContentVerticalAlignment.center;
            
            self.isOpaque = false
            self.layer.cornerRadius = self.frame.size.width / 2;
            self.layer.borderWidth = self.borderWidth;
            self.layer.borderColor = self.borderColor.cgColor;
            self.clipsToBounds = true

            self.setTitle("+", for: UIControlState())
            self.setTitleColor(self.textColor, for: UIControlState())
            self.titleLabel?.font = UIFont.systemFont(ofSize: 40)
            self.contentEdgeInsets.bottom = 5
            
            initialized = true
        }
    }
}
