//
//  ProjectImageBeforeAfter.swift
//  Prestige
//
//  Created by Joel Teply on 7/22/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import Kingfisher

@IBDesignable
class ProjectImageBeforeAfterView : UIView {
    
    let beforeImageView = UIImageView()
    let afterImageView = UIImageView()
    let divider = UIView()
    
    let beforeAfterOverlayHeight:CGFloat = 40
    let beforeAfterOverlay = UIView()
    let beforeLabel = UILabel()
    let afterLabel = UILabel()
    
    fileprivate var dividerThickness:CGFloat = 5
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        if beforeImageView.superview == nil {
            //first run with parent, aka initialization
            beforeImageView.contentMode = .scaleAspectFill
            beforeImageView.clipsToBounds = true
            self.addSubview(beforeImageView)
            afterImageView.contentMode = .scaleAspectFill
            afterImageView.clipsToBounds = true
            self.addSubview(afterImageView)
            
            self.addSubview(divider);
            
            beforeAfterOverlay.backgroundColor = UIColor.black.withAlphaComponent(0.5)
            
            beforeLabel.text = "Before"
            beforeLabel.textAlignment = .center
            beforeAfterOverlay.addSubview(beforeLabel)
            
            afterLabel.text = "After"
            afterLabel.textAlignment = .center
            beforeAfterOverlay.addSubview(afterLabel)
            
            self.addSubview(beforeAfterOverlay)
        }
        
        #if TARGET_INTERFACE_BUILDER
            beforeImageView.backgroundColor = UIColor.lightGrayColor()
            afterImageView.backgroundColor = UIColor.lightGrayColor()
        #else
            if let image = image {
                afterImageView.isHidden = !image.previewImageExists
            }
        #endif
        
        let hasOneImage = afterImageView.isHidden
        
        divider.isHidden = hasOneImage
        divider.backgroundColor = self.tintColor
        divider.frame = CGRect(x: (self.frame.size.width - dividerThickness) / 2, y: 0, width: dividerThickness, height: self.frame.size.height)
        
        beforeAfterOverlay.frame = CGRect(x: 0, y: self.frame.size.height - beforeAfterOverlayHeight, width: self.frame.size.width, height: beforeAfterOverlayHeight)
        beforeAfterOverlay.isHidden = hasOneImage
        beforeLabel.textColor = self.tintColor
        afterLabel.textColor = self.tintColor
        
        if !hasOneImage {
            print(self.frame.size.width)
            beforeImageView.frame = CGRect(x: 0, y: 0, width: self.frame.size.width / 2, height: self.frame.size.height)
            beforeLabel.frame = CGRect(x: beforeImageView.frame.origin.x, y: 0, width: beforeImageView.frame.width, height: beforeAfterOverlay.frame.height)
            
            afterImageView.frame = CGRect(x: self.frame.size.width / 2, y: 0, width: self.frame.size.width / 2, height: self.frame.size.height)
            afterLabel.frame = beforeLabel.frame
            afterLabel.frame.origin.x = self.frame.size.width / 2
        } else {
            beforeImageView.frame = self.frame
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
    
}
