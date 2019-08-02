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

class ARVRViewController: UIViewController, ProductSelectionDelegate, CBRemodelingViewDelegate {
    
    @IBOutlet weak var vrView: CBRemodelingView!
    @IBOutlet weak var rightMenuConstraint: NSLayoutConstraint!
    @IBOutlet weak var rightMenuView: UIView!
    @IBOutlet weak var rightMenuTitleButton: UIButton!
    @IBOutlet weak var rightMenuIconButton: UIButton!
    
    var isVR = false
    
    override func viewDidLoad() {
        super.viewDidLoad()

        self.vrView.delegate = self
        self.rightMenuConstraint.constant = 0
        // Do any additional setup after loading the view.
        
        showSidebar(false, animated: false)
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
        
        
        if let prod = Product.first() {
//            let deadlineTime = DispatchTime.now() + .seconds(3)
//            DispatchQueue.main.asyncAfter(deadline: deadlineTime) {
//                self.productColorChanged(product: prod, color: prod.colors.first!);
//            }
            self.productColorChanged(product: prod, color: prod.colors.first!);
        }
    }
    
    @IBAction func toggleSidebar(_ sender: Any) {
        showSidebar(self.rightMenuView.isHidden)
    }
    
    func showSidebar(_ show:Bool, animated:Bool = true) {
        
        let endValue = show ? 0 : -self.rightMenuView.bounds.size.width
        let endTitle = show ? "ic-next" : "ic-previous"
        
        if (animated) {
            self.rightMenuView.isHidden = false
            self.rightMenuTitleButton.isHidden = true
            
            UIView.animate(withDuration: 0.3, delay: 0.0, options: .curveEaseIn, animations: {
                self.rightMenuConstraint.constant = endValue
                self.rightMenuView.superview?.layoutIfNeeded()
            }, completion: { finished in
                self.rightMenuView.isHidden = !show
                self.rightMenuTitleButton.isHidden = show
                self.rightMenuIconButton.setImage(UIImage(named: endTitle), for: .normal)
            })
        } else {
            self.rightMenuConstraint.constant = endValue
            self.rightMenuView.isHidden = !show
            self.rightMenuTitleButton.isHidden = show
            self.rightMenuIconButton.setImage(UIImage(named: endTitle), for: .normal)
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
        if let nav = segue.destination as? UINavigationController,
            let controller = nav.viewControllers.first as? ProductSelectionViewController {
            controller.delegate = self
            controller.isVR = self.isVR
        }
    }
    
    func productColorChanged(product: Product, color: ProductColor) {
        let hud = JGProgressHUD(style: .dark)
        hud.indicatorView = JGProgressHUDRingIndicatorView()
        hud.textLabel.text = "Downloading"
        var hudShown = false
        var lastProgress:Float = 0.0
        
        self.showSidebar(false)
        
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
    
    func productInstallationChanged(product: Product, method: InstallationMethod) {
        self.vrView.sendUnityCommand("updateInstallation", json: "{\"product\":\(product.jsonString), \"method\":\(method.jsonString)}")
        
        print("Selected product \(product.name), method \(method.name)")
        showSidebar(false)
    }
    
    func sceneLocationChanged(location: SceneLocation) {
        self.vrView.sendUnityCommand("updateLocation", json: "{\"location\":\(location.jsonString)}")
        
        print("Selected scene:\(location.jsonString)")
        showSidebar(false)
    }
    
    func uploadDiagnosticImage(atPath path:String) {
        DataController.sharedInstance.uploadDiagnosticImage(path);
    }
}
