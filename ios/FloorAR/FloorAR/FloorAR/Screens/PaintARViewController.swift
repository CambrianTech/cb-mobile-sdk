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
    
    @IBOutlet weak var captureButton: UIButton!
    @IBOutlet weak var paintButton: UIButton!
    @IBOutlet weak var eraserButton: UIButton!
    
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
    }
    
    override func viewDidAppear(_ animated: Bool) {
       if UIImagePickerController.isSourceTypeAvailable(.camera) {
            self.startRunning()
       } else {
           AVCaptureDevice.requestAccess(for: .video) { response in
               if response {
                   self.startRunning()
               } else {
                   
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
    
    func handleToolMode() {
        self.paintButton.isSelected = self.arView.toolMode == .paintbrush
        self.eraserButton.isSelected = self.arView.toolMode == .eraser
        self.captureButton.isSelected = self.arView.isLive
    }
    
    @IBAction func closeClicked(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }

    func startRunning() {
        
        self.arView.startRunning()
        self.arView.toolMode = .fill
        self.paintButton.isHidden = true
        self.eraserButton.isHidden = true
        
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

    func paintSelected(_ paint: BrandItem) {
        self.paint.color = paint.color
    }
    
    func categorySelected(_ category: BrandCategory) {
        history?.currentItem = category
        swatches?.category = category
    }
    
    func historyChanged(_ current:HistoryItem?) {
        if let category = current as? BrandCategory {
            print("historyChanged \(category.getName())")
            swatches?.category = category
        }
    }
    
    func getRootHistoryName() -> String? {
        return nil
    }
}
