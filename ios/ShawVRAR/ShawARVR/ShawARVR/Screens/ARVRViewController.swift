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

class ARVRViewController: UIViewController, CBRemodelingViewDelegate, ProductSelectionDelegate {
    
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
    
    func startRunning() {
        self.vrView.startRunning(true, unityScene: isVR ? "Scenes/ShawVR" : nil)
        
        if (!isVR) {
            self.vrView.scene.appendAsset(CBRemodelingFloor());
        }
        
        if let prod = Product.random() {
            self.productColorChanged(product: prod, color: prod.colors.first!);
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "floor-scroller" {
            if let destVC = segue.destination as? FloorCollectionView {
                destVC.delegate = self
            }
        }
    }
    
    func productColorChanged(product: Product, color: ProductColor) {
        let hud = JGProgressHUD(style: .dark)
        hud.indicatorView = JGProgressHUDRingIndicatorView()
        hud.textLabel.text = "Downloading"
        var hudShown = false
        var lastProgress:Float = 0.0
                
        color.loadData(progress: { (receivedSize, expectedSize) in
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
                self.vrView.sendUnityCommand("updateMaterial", json: "{\"product\":\(product.jsonString), \"color\":\(color.jsonString)}")
                print("Selected product \(product.name), color \(color.name)")
                
            } else {
                //Fail
            }
        }
        
    }
    
    func uploadDiagnosticImage(atPath path:String) {
        DataController.sharedInstance.uploadDiagnosticImage(path);
    }
}
