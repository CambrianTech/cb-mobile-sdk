//
//  ImageRenderer.swift
//  Prestige
//
//  Created by Joel Teply on 8/3/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

class ImageRenderer {
    var image:VisualizerImage
    init(image:VisualizerImage) {
        self.image = image
    }
    
    func getRenderedImage(_ ontoImage: UIImage?=nil) -> UIImage? {
        
        var ba = ontoImage
        if ontoImage == nil {
            ba = self.image.beforeAfterImage
        }
        
        guard let _ = self.image.project, let beforeAfter = ba
            else {
            return nil
        }
        
        let assets = image.assets
        
        let perSampleHeight = 0.12 * beforeAfter.size.height
        
        let fullHeight = beforeAfter.size.height + perSampleHeight * CGFloat(assets.count)
        let margin = 0.03 * beforeAfter.size.width
        let textLeftMargin = 1.2 * margin
        let textTopMargin = 0.5 * margin
        
        UIGraphicsBeginImageContext(CGSize(width: beforeAfter.size.width, height: fullHeight))
        
        //draw image
        beforeAfter.draw(in: CGRect(origin: CGPoint.zero, size: beforeAfter.size))
        
        //draw watermark
        if let watermarkImage = UIImage(named: "share-watermark") {
            let watermarkDesiredWidth = 0.3 * beforeAfter.size.width
            let watermarkDesiredHeight = 0.08 * beforeAfter.size.width
            let watermarkScalingFactorW = watermarkDesiredWidth / watermarkImage.size.width
            let watermarkScalingFactorH = watermarkDesiredHeight / watermarkImage.size.height
            let watermarkScalingFactor = min(watermarkScalingFactorW, watermarkScalingFactorH)
            
            let watermarkSize = CGSize(width: watermarkScalingFactor * watermarkImage.size.width, height: watermarkScalingFactor * watermarkImage.size.height)
            watermarkImage.draw(in: CGRect(origin: CGPoint(x: margin, y: beforeAfter.size.height - margin - watermarkSize.height), size: watermarkSize))
        }
        
        
        if assets.count > 0 {
            let font = UIFont.boldSystemFont(ofSize: 0.25 * perSampleHeight)
            let vendorIDFont = UIFont.boldSystemFont(ofSize: 0.2 * perSampleHeight)
            
            var yPos = beforeAfter.size.height
            for asset in assets {
                let item = asset.getItem()!
                
                let colorRect = CGRect(x: 0, y: yPos, width: beforeAfter.size.width, height: perSampleHeight)

                item.color.setFill()
                UIRectFill(colorRect)
                
                let textRect = CGRect(x: textLeftMargin, y: yPos + textTopMargin,
                                      width: beforeAfter.size.width - textLeftMargin, height: perSampleHeight - textTopMargin)
                
                let needsDarkText = item.needsDarkText()
                let nameTextColor = needsDarkText ? UIColor.darkText : UIColor.white
                let codeTextColor = needsDarkText ? UIColor.darkText : UIColor.white
                
                let text = NSMutableAttributedString(string: "Product: \(item.name)", attributes:
                    [NSAttributedStringKey.font: font, NSAttributedStringKey.foregroundColor: nameTextColor])
                text.append(NSAttributedString(string: "\n"))
                text.append(NSAttributedString(string: "ID: \(item.storeID)", attributes:
                    [NSAttributedStringKey.font: vendorIDFont, NSAttributedStringKey.foregroundColor: codeTextColor]))
                
                text.draw(in: textRect)
                
                yPos += perSampleHeight
            }
        }
        
        let watermarkedImage = UIGraphicsGetImageFromCurrentImageContext()
        UIGraphicsEndImageContext()
        
        return watermarkedImage
    }

}
