//
//  FloatingItemButton.swift
//  HarmonyApp
//
//  Created by joseph on 5/1/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import Kingfisher

class FloatingItemButton: UIButton {
    
    
    var item: BrandItem? {
        didSet {
            if let item = item {
                update(item)
            }
        }
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        self.layer.cornerRadius = self.frame.height / 2
        self.isUserInteractionEnabled = true
    }
    
    func update(_ item: BrandItem) {
        if item.hasThumbnail {
            self.setTexture(item.getThumbnailPath())
            self.enableLogo(false)
        } else {
            self.setColor(item.color)
            self.enableLogo(true)
        }
        self.isHidden = false
    }
    
    func setColor(_ color: UIColor) {
        self.layer.contents = nil
        self.layer.backgroundColor = color.cgColor
    }
    
    func enableLogo(_ enable: Bool) {
        if(enable) {
            self.setImage(nil, for: .normal)
        } else {
            self.setImage(nil, for: .normal)
        }
    }
    
    func setTexture(_ url: URL?) {
        if let url = url {
            //print("url: \(url)")
            let size = self.frame.height
            let processor = ResizingImageProcessor(referenceSize: CGSize(width: size, height: size), mode: .none) >>
                RoundCornerImageProcessor(cornerRadius: CGFloat(size/2))
            let scale = UIScreen.main.scale
            KingfisherManager.shared.retrieveImage(with: url, options: [.processor(processor), .scaleFactor(scale)], progressBlock: nil, completionHandler: {
                image, error, cacheType, imageURL in
                
                self.layer.contents = image?.cgImage
            })
        }
    }
}
