//
//  HomeViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import AVFoundation

class HomeViewController: UIViewController, UIImagePickerControllerDelegate, UINavigationControllerDelegate {
    
    let imagePicker =  UIImagePickerController()
    let alertController = UIAlertController()
    var photoToLoad:UIImage?
    var hasCameraAccess = false

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
    
    func checkCameraAccess() {
        switch AVCaptureDevice.authorizationStatus(for: .video) {
            case .denied:
                print("Denied, request permission from settings")
                self.presentCameraSettings()
            case .restricted:
                print("Restricted, device owner must approve")
                self.presentCameraSettings()
            case .authorized:
                print("Authorized, proceed")
                self.hasCameraAccess = true
            case .notDetermined:
                AVCaptureDevice.requestAccess(for: .video) { success in
                    if success {
                        print("Permission granted, proceed")
                        self.hasCameraAccess = true
                    } else {
                        print("Permission denied")
                    }
                }
            @unknown default: print("unknown status")
        }
    }

    func presentCameraSettings() {
        let alertController = UIAlertController(title: "Camera is required",
                                      message: "In order to proceed, you must modify the settings of this app to allow camera access.",
                                      preferredStyle: .alert)
        alertController.addAction(UIAlertAction(title: "Cancel", style: .default))
        alertController.addAction(UIAlertAction(title: "Settings", style: .cancel) { _ in
            UIApplication.shared.open(URL(string: UIApplication.openSettingsURLString)!, options: [:], completionHandler: nil)
        })
        
        if UI_USER_INTERFACE_IDIOM() == .pad {
            addActionSheetForiPad(actionSheet: alertController)
        }
        
        present(alertController, animated: true)
    }
    
    func visualizeImage(_ isCamera:Bool) {
        if (!self.hasCameraAccess) {
            presentCameraSettings()
            return
        }
        imagePicker.delegate = self
        imagePicker.mediaTypes = ["public.image"]
        //imagePicker.allowsEditing = true
        imagePicker.sourceType = isCamera ? .camera : .photoLibrary
        
        if UIDevice.current.userInterfaceIdiom == .pad {
            alertController.popoverPresentationController?.sourceView = self.view
            alertController.popoverPresentationController?.sourceRect = self.view.bounds
            alertController.popoverPresentationController?.permittedArrowDirections = [.down, .up]
        }
        
        present(imagePicker, animated: true)
    }
    
    internal func imagePickerController(_ picker: UIImagePickerController, didFinishPickingMediaWithInfo info: [UIImagePickerController.InfoKey : Any]) {

        let desiredSize = CGSize(width: 1024,height: 1024)
        if let editedImage = info[.editedImage] as? UIImage {
            photoToLoad = resizeImage(image: editedImage, targetSize: desiredSize)
        } else if let originalImage = info[.originalImage] as? UIImage {
            photoToLoad = resizeImage(image: originalImage, targetSize: desiredSize)
        }
        picker.dismiss(animated: true, completion: {
            self.performSegue(withIdentifier: "visualize", sender: self)
        })
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
