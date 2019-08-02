//
//  RoundImage.swift
//  HarmonyApp
//
//  Created by joseph on 5/1/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import Kingfisher


@IBDesignable
class RoundImage : UIImageView {
    
    override func layoutSubviews() {
        super.layoutSubviews()
        self.layer.cornerRadius = self.frame.height / 2
        self.isUserInteractionEnabled = true
    }
}

class RoundImageBordered : RoundImage {
    
    let maskLayer = CAShapeLayer()
    let imageLayer = CALayer()
    
    var isSelected: Bool = false {
        didSet {
            if(isSelected) {
                self.layer.backgroundColor = UIColor.white.withAlphaComponent(0.5).cgColor
            } else {
                self.layer.backgroundColor = UIColor.black.withAlphaComponent(0.5).cgColor
            }
        }
    }
    
    
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        addLayer()
    }
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        addLayer()
    }
    
    func addLayer() {
        let path = UIBezierPath(arcCenter: CGPoint(x: frame.width/2, y: frame.height/2),
                                radius: (frame.width / 2) - 4,
                                startAngle: 0,
                                endAngle: CGFloat(2.0 * .pi),
                                clockwise: true)
        
        let size = self.frame.width
        
        maskLayer.path = path.cgPath
        maskLayer.cornerRadius = maskLayer.frame.width / 2
        maskLayer.fillColor = UIColor.green.cgColor
        
        imageLayer.frame = CGRect(x: 0, y:0, width: size, height: size)
        imageLayer.mask = maskLayer
        
        self.layer.addSublayer(imageLayer)
        
        self.layer.backgroundColor = UIColor.black.withAlphaComponent(0.5).cgColor
    }
    
    func setColor(_ color: UIColor) {
        imageLayer.contents = nil
        imageLayer.backgroundColor = color.cgColor
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
                
                self.imageLayer.contents = image?.cgImage
            })
        }
    }
}
