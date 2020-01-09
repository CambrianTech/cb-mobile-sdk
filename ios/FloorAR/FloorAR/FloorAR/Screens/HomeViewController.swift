//
//  HomeViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import AVFoundation

class HomeViewController: CameraViewController {
    
    var photoToLoad:UIImage?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        ProductCategory.sync()
        SceneLocation.sync()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        checkCameraAccess()
    }
    
    @IBAction func showLiveVisualizer(_ sender: Any) {
        if (self.hasCameraAccess) {
            self.performSegue(withIdentifier: "live-visualizer", sender: self)
        } else {
            self.presentCameraSettings()
        }
    }
    
    @IBAction func roomPhotoClicked(_ sender: Any) {
        let optionMenu = UIAlertController(title: nil, message: "What kind of photo do you need?", preferredStyle: .actionSheet)
            
        optionMenu.addAction(UIAlertAction(title: "Take Picture", style: .default, handler:{ (UIAlertAction) in
            self.visualizeImage(true)
        }))
        
        optionMenu.addAction(UIAlertAction(title: "Photo Library", style: .default, handler:{ (UIAlertAction) in
            self.visualizeImage(false)
        }))
        
        optionMenu.addAction(UIAlertAction(title: "Sample Room", style: .default, handler:{ (UIAlertAction) in
            self.performSegue(withIdentifier: "samples", sender: self)
        }))
        
        optionMenu.addAction(UIAlertAction(title: "Cancel", style: .cancel))
        
        if UI_USER_INTERFACE_IDIOM() == .pad {
            addActionSheetForiPad(actionSheet: optionMenu)
        }
        
        self.present(optionMenu, animated: true, completion: nil)
    }
    
    override func pickedImage(image:UIImage) {
        self.photoToLoad = image
        self.performSegue(withIdentifier: "visualize", sender: self)
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
       if segue.identifier == "visualize" {
            if let vc = segue.destination as? PhotoViewController {
                vc.photoToLoad = self.photoToLoad
                self.photoToLoad = nil
            }
        }
    }
}
