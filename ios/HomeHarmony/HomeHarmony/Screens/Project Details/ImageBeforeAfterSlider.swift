//
//  ImageBeforeAfterSlider.swift
//  HarmonyApp
//
//  Created by Jaremy Longley on 10/30/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import UIKit
import Foundation
import Kingfisher

@IBDesignable
public class ImageBeforeAfterSlider: UIView {
    
    fileprivate var leading: NSLayoutConstraint!
    fileprivate var originRect: CGRect!
    
    @IBInspectable
    public var image1: UIImage = UIImage() {
        didSet {
            afterImageView.image = image1
        }
    }

    @IBInspectable
    public var image2: UIImage = UIImage() {
        didSet {
            beforeImageView.image = image2
        }
    }

    @IBInspectable
    public var thumbColor: UIColor = UIColor.white {
        didSet {
            thumb.backgroundColor = thumbColor
        }
    }
    
    var image: VisualizerImage? {
        didSet {
            if let image = image {
                beforeImageView.kf.setImage(with: image.originalImagePath)
                if image.previewImageExists {
                    let resource = ImageResource(downloadURL: image.previewImagePath!, cacheKey: String(describing: image.modified))
                    afterImageView.kf.setImage(with: resource)
                }
            }
            self.setNeedsDisplay()
        }
    }
    
    fileprivate lazy var beforeImageView: UIImageView = {
        let iv = UIImageView()
        iv.translatesAutoresizingMaskIntoConstraints = false
        iv.contentMode = .scaleAspectFill
        iv.clipsToBounds = true
        return iv
    }()
    
    fileprivate lazy var afterImageView: UIImageView = {
        let iv = UIImageView()
        iv.translatesAutoresizingMaskIntoConstraints = false
        iv.contentMode = .scaleAspectFill
        iv.clipsToBounds = true
        return iv
    }()
    
    fileprivate lazy var afterImageWrapper: UIView = {
        let v = UIView()
        v.translatesAutoresizingMaskIntoConstraints = false
        v.layer.borderColor = UIColor.black.cgColor
        v.layer.borderWidth = 1
        v.clipsToBounds = true
        return v
    }()
    
    fileprivate lazy var thumbWrapper: UIView = {
        let v = UIView()
        v.translatesAutoresizingMaskIntoConstraints = false
        v.clipsToBounds = true
        return v
    }()
    
    fileprivate lazy var thumb: UIView = {
        let view = UIView()
        view.backgroundColor = UIColor.white
        view.translatesAutoresizingMaskIntoConstraints = false
        view.clipsToBounds = true
        return view
    }()
    
    lazy fileprivate var setupLeadingAndOriginRect: Void = {
        self.leading.constant = self.frame.width / 2
        self.layoutIfNeeded()
        self.originRect = self.afterImageWrapper.frame
    }()
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        initialize()
    }
    
    required public init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        initialize()
    }
    
    override public func layoutSubviews() {
        super.layoutSubviews()
        _ = setupLeadingAndOriginRect
    }
}

extension ImageBeforeAfterSlider {
    fileprivate func initialize() {
        
        afterImageWrapper.addSubview(afterImageView)
        addSubview(beforeImageView)
        addSubview(afterImageWrapper)
        
        thumbWrapper.addSubview(thumb)
        addSubview(thumbWrapper)
        
        NSLayoutConstraint.activate([
            beforeImageView.topAnchor.constraint(equalTo: topAnchor, constant: 0),
            beforeImageView.bottomAnchor.constraint(equalTo: bottomAnchor, constant: 0),
            beforeImageView.trailingAnchor.constraint(equalTo: trailingAnchor, constant: 0),
            beforeImageView.leadingAnchor.constraint(equalTo: leadingAnchor, constant: 0)
            ])
        
        leading = afterImageWrapper.leadingAnchor.constraint(equalTo: leadingAnchor, constant: 0)
        
        NSLayoutConstraint.activate([
            afterImageWrapper.topAnchor.constraint(equalTo: topAnchor, constant: 0),
            afterImageWrapper.bottomAnchor.constraint(equalTo: bottomAnchor, constant: 0),
            afterImageWrapper.trailingAnchor.constraint(equalTo: trailingAnchor, constant: 0),
            leading
            ])
        
        NSLayoutConstraint.activate([
            afterImageView.topAnchor.constraint(equalTo: afterImageWrapper.topAnchor, constant: 0),
            afterImageView.bottomAnchor.constraint(equalTo: afterImageWrapper.bottomAnchor, constant: 0),
            afterImageView.trailingAnchor.constraint(equalTo: afterImageWrapper.trailingAnchor, constant: 0)
            ])
        
        NSLayoutConstraint.activate([
            thumbWrapper.topAnchor.constraint(equalTo: afterImageWrapper.topAnchor, constant: 0),
            thumbWrapper.bottomAnchor.constraint(equalTo: afterImageWrapper.bottomAnchor, constant: 0),
            thumbWrapper.leadingAnchor.constraint(equalTo: afterImageWrapper.leadingAnchor, constant: -20),
            thumbWrapper.widthAnchor.constraint(equalToConstant: 40)
            ])
        
        NSLayoutConstraint.activate([
            thumb.centerXAnchor.constraint(equalTo: thumbWrapper.centerXAnchor, constant: 0),
            thumb.centerYAnchor.constraint(equalTo: thumbWrapper.centerYAnchor, constant: 0),
            thumb.widthAnchor.constraint(equalTo: thumbWrapper.widthAnchor, multiplier: 1),
            thumb.heightAnchor.constraint(equalTo: thumbWrapper.widthAnchor, multiplier: 1)
            ])
        
        leading.constant = frame.width / 2
        
        thumb.layer.cornerRadius = 20
        afterImageView.widthAnchor.constraint(equalTo: widthAnchor, multiplier: 1).isActive = true
        
        let tap = UIPanGestureRecognizer(target: self, action: #selector(gesture(sender:)))
        thumbWrapper.isUserInteractionEnabled = true
        thumbWrapper.addGestureRecognizer(tap)
    }
    
    
    @objc func gesture(sender: UIPanGestureRecognizer) {
        let translation = sender.translation(in: self)
        switch sender.state {
        case .began, .changed:
            var newLeading = originRect.origin.x + translation.x
            newLeading = max(newLeading, 20)
            newLeading = min(frame.width - 20, newLeading)
            leading.constant = newLeading
            layoutIfNeeded()
        case .ended, .cancelled:
            originRect = afterImageWrapper.frame
        default: break
        }
    }
}


