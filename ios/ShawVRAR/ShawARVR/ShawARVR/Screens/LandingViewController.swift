//
//  ViewController.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/15/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit
import AVFoundation
import Digger
import Zip

class LandingViewController: UIViewController {

    var isVR = false
    
    @IBOutlet weak var showARButton: RoundButton!
    @IBOutlet weak var showVRButton: RoundButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        // Hide the navigation bar on the this view controller
        self.navigationController?.setNavigationBarHidden(true, animated: animated)
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        self.navigationController?.setNavigationBarHidden(false, animated: animated)
    }
    
    @IBAction func showAR(_ sender: Any) {
        isVR = false
        proceedWithCameraAccess(identifier:"showARVR")
    }
    
    @IBAction func showVR(_ sender: Any) {
        isVR = true
        proceedWithCameraAccess(identifier:"showARVR")
    }
    
    func proceedWithCameraAccess(identifier:String) {
        if AVCaptureDevice.authorizationStatus(for:  AVMediaType.video) == .authorized {
            self.performSegue(withIdentifier: identifier, sender: nil)
            return
        }
        AVCaptureDevice.requestAccess(for: AVMediaType.video) { success in
            if success {
                DispatchQueue.main.async {
                    self.performSegue(withIdentifier: identifier, sender: nil)
                }
            } else {
                let alert = UIAlertController(title: "Camera", message: "Camera access is required to use this application", preferredStyle: UIAlertControllerStyle.alert)
                
                alert.addAction(UIAlertAction(title: "OK", style: .default, handler: { action in
                    UIApplication.shared.open(URL(string:UIApplicationOpenSettingsURLString)!)
                }))
                self.present(alert, animated: true)
            }
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let controller = segue.destination as? ARVRViewController {
            controller.isVR = isVR
        }
    }
    
    override var prefersStatusBarHidden: Bool {
        get {
            return true
        }
    }
}

