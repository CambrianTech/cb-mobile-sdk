//
//  VRViewController.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/15/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit
import AVFoundation
import JGProgressHUD

class ARViewController: UIViewController, CBRemodelingViewDelegate, ProductSelectionDelegate {
    
    @IBOutlet weak var vrView: CBRemodelingView!
    
    var isVR = false
    
    override func viewDidLoad() {
        super.viewDidLoad()

        self.vrView.delegate = self
        // Do any additional setup after loading the view.
    }

    override func viewDidAppear(_ animated: Bool) {
        if UIImagePickerController.isSourceTypeAvailable(.camera) {
            startRunning()
        } else {
            AVCaptureDevice.requestAccess(for: AVMediaType.video) { response in
                if response {
                    self.startRunning()
                } else {
                    
                }
            }
        }
    }
    
    @IBAction func closeClicked(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }
    
    func startRunning() {
        self.vrView.startRunning(true, unityScene: isVR ? "Scenes/ShawVR" : nil)
        
        if (!isVR) {
            self.vrView.scene.appendAsset(CBRemodelingFloor());
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "product-navigation" {
            if let productSelector = segue.destination as? ProductSelectionView {
                productSelector.delegate = self
            }
        }
    }
    
    func productSelectionViewDidLoad(psv:ProductSelectionView) {
        psv.swatchScroller.backgroundColor = UIColor.darkGray
    }
    
    func productColorChanged(product: Product, color: ProductColor) {
        let hud = JGProgressHUD(style: .dark)
        hud.indicatorView = JGProgressHUDRingIndicatorView()
        hud.textLabel.text = "Downloading"
        var hudShown = false
        var lastProgress:Float = 0.0
                
        color.defaultVariation.loadData(progress: { (receivedSize, expectedSize) in
            let amount = 0.9 * Float(receivedSize) / max(Float(expectedSize), 1.0)
            //print("Progress Bar: \(receivedSize) Total: \(expectedSize), Pecent: \(amount * 100.0)")

            if amount > lastProgress {
                lastProgress = amount
                DispatchQueue.main.async {
                    hud.progress = amount
                    if (!hudShown) {
                        hudShown = true
                        hud.show(in: self.view)
                    }
                }
            }

        })
        { (completed) in
            if (hudShown) {
                DispatchQueue.main.async {
                    hud.progress = 1.0
                    hud.dismiss(afterDelay: 0.5)
                }
            }
            if (completed) {
                DispatchQueue.main.asyncAfter(
                    deadline: DispatchTime.now() + Double(Int64(hudShown ? 0.5 * Double(NSEC_PER_SEC) : 0)) / Double(NSEC_PER_SEC), execute: {
                        self.vrView.sendUnityCommand("updateMaterial", json: "{\"product\":\(product.jsonCommand), \"color\":\(color.jsonCommand)}")
                        print("Selected product \(product.name), color \(color.name)")
                })
            } else {
                //Fail
            }
        }
        
    }
}
