//
//  PaintARViewController.swift
//  DesignUp
//
//  Created by Joel Teply on 1/22/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import UIKit
import CambrianAR
import AVFoundation

class PaintARViewController: UIViewController, CBARRemodelingViewDelegate, PaintSelectionDelegate, HistorySelectionDelegate {
    @IBOutlet weak var arView: CBARRemodelingView!
    @IBOutlet weak var paintNameLabel: UILabel!
    
    @IBOutlet weak var captureButton: UIButton!
    @IBOutlet weak var paintButton: UIButton!
    @IBOutlet weak var eraserButton: UIButton!
    @IBOutlet weak var shareButton: UIButton!
    
    var paint = CBARRemodelingPaint(assetID: "Paint")
    var category:BrandCategory? {
        get {
            return DataSource.current.paintsRealm.objects(BrandCategory.self).filter({$0.parentCategory == nil}).first
        }
    }

    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.arView.delegate = self;
        
        self.arView.isAREnabled = true
        self.paint.color = UIColor.clear
        
        self.paintButton.isHidden = true
        self.eraserButton.isHidden = true
        self.shareButton.isHidden = true
        self.paintNameLabel.isHidden = true
    }
    
    override func viewDidAppear(_ animated: Bool) {
       if UIImagePickerController.isSourceTypeAvailable(.camera) {
            self.startRunning()
       } else {
           AVCaptureDevice.requestAccess(for: .video) { response in
               if response {
                   self.startRunning()
               }
           }
       }
    }
    
    @IBAction func captureClicked(_ sender: Any) {
        
        if (self.arView.isLive) {
            self.arView.captureCurrentState()
            self.arView.toolMode = .paintbrush
            self.paintButton.isHidden = false
            self.eraserButton.isHidden = false
            self.shareButton.isHidden = self.lastPaintName == nil
        } else {
            startRunning()
        }
        
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            self.handleToolMode()
        }
    }
    
    @IBAction func paintbrushClicked(_ sender: Any) {
        self.arView.toolMode = .paintbrush
        handleToolMode()
    }
    
    @IBAction func eraserClicked(_ sender: Any) {
        self.arView.toolMode = .eraser
        handleToolMode()
    }
    
    @IBAction func shareClicked(_ sender: Any) {
        
        if self.paint.color != .clear, let text = self.lastPaintName {
            self.arView.getImagePreview { (image) in
                self.share(image:image, text:text, color:self.paint.color)
            }
        }
    }
    
    func handleToolMode() {
        self.paintButton.isSelected = self.arView.toolMode == .paintbrush
        self.eraserButton.isSelected = self.arView.toolMode == .eraser
        self.captureButton.isSelected = self.arView.isLive
    }
    
    func share(image:UIImage, text:String, color:UIColor) {
        let textColor = UIColor.white
        let textFont = UIFont(name: "Helvetica Bold", size: 30)!

        UIGraphicsBeginImageContextWithOptions(image.size, false, 1.0)
        
        //background
        image.draw(in: CGRect(origin: CGPoint.zero, size: image.size))
        
        UIColor(red: 0, green: 0, blue: 0, alpha: 0.7).setFill()
        UIRectFillUsingBlendMode(CGRect(origin: CGPoint(x: 0, y: image.size.height - 70), size: CGSize(width: image.size.width, height: 70)), .multiply)
        
        let swatchRect = CGRect(origin: CGPoint(x: 20, y: image.size.height - 170), size: CGSize(width: 150, height: 150))
        color.setFill()
        UIRectFill(swatchRect)
        
        UIColor(red: 0, green: 0, blue: 0, alpha: 0.8).setFill()
        UIRectFrameUsingBlendMode(swatchRect, .multiply)

        //draw brand info text
        let textFontAttributes = [
            NSAttributedString.Key.font: textFont,
            NSAttributedString.Key.foregroundColor: textColor,
            ] as [NSAttributedString.Key : Any]
        
        let rect = CGRect(origin: CGPoint(x: 200, y: image.size.height - 55), size: CGSize(width: image.size.width, height: 50))
        text.draw(in: rect, withAttributes: textFontAttributes)

        let brandImage = UIGraphicsGetImageFromCurrentImageContext()
        UIGraphicsEndImageContext()
        
        if let image = brandImage {
            let activityViewController = UIActivityViewController(activityItems: [image] , applicationActivities: nil)
            activityViewController.popoverPresentationController?.sourceView = self.view
            self.present(activityViewController, animated: true, completion: nil)
        }
    }
    
    @IBAction func closeClicked(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }

    func startRunning() {
        
        self.arView.startRunning()
        self.arView.toolMode = .fill
        self.paintButton.isHidden = true
        self.eraserButton.isHidden = true
        self.shareButton.isHidden = true
        
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            if (self.arView.scene.assets.count == 0) {
                self.arView.scene.appendAsset(self.paint)
            }
            self.handleToolMode()
        }
    }
    
    private var swatches:SwatchSelectorController?
    private var history:HistorySelectionView?
    
    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let vc = segue.destination as? SwatchSelectorController {
            vc.delegate = self
            vc.category = self.category
            swatches = vc
        } else if let vc = segue.destination as? HistorySelectionView {
            vc.delegate = self
            vc.currentItem = self.category
            history = vc
        }
    }

    var lastPaintName:String? = nil
    func paintSelected(_ paint: BrandItem) {
        self.paint.color = paint.color
        self.paintNameLabel.isHidden = false
        if let category = swatches?.category?.parentCategory {
            self.lastPaintName = "\(category.name) - \(paint.name)"
            self.shareButton.isHidden = self.arView.isLive || self.lastPaintName == nil
            self.paintNameLabel.text = self.lastPaintName
        }
    }
    
    func categorySelected(_ category: BrandCategory) {
        history?.currentItem = category
        swatches?.category = category
        self.paintNameLabel.isHidden = false
        self.paintNameLabel.text = category.name
        if let brand = category.parentCategory, let _ = category.parentCategory?.parentCategory  {
            self.paintNameLabel.text = "\(brand.name) - \(category.name)"
        } else {
            self.paintNameLabel.text = category.name
        }
    }
    
    func historyChanged(_ current:HistoryItem?) {
        if let category = current as? BrandCategory {
            swatches?.category = category
        }
    }
    
    func getRootHistoryName() -> String? {
        return nil
    }
}
