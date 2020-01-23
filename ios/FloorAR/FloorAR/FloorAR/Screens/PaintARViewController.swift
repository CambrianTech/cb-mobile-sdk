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

class PaintARViewController: UIViewController, CBARRemodelingViewDelegate {
    
    @IBOutlet weak var arView: CBARRemodelingView!
    var paint = CBARRemodelingPaint(assetID: "Paint")

    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.arView.delegate = self;
        self.arView.isAREnabled = true
        self.paint.color = UIColor.blue
        self.arView.toolMode = .fill
    }
    
    override func viewDidAppear(_ animated: Bool) {
           if UIImagePickerController.isSourceTypeAvailable(.camera) {
               startRunning()
           } else {
               AVCaptureDevice.requestAccess(for: .video) { response in
                   if response {
                       self.startRunning()
                   } else {
                       
                   }
               }
           }
       }

    func startRunning() {
        self.arView.startRunning()
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            self.arView.scene.appendAsset(self.paint)
            self.paint.color = UIColor.red
        }
    }
    
    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        
    }

}
