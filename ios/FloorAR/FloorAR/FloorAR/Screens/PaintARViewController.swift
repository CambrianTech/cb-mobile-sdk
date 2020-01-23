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

    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.arView.delegate = self;
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
    }
    
    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        
    }

}
