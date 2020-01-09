//
//  PhotoViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 1/8/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import UIKit
import AVFoundation

class CameraViewController: UIViewController, UIImagePickerControllerDelegate, UINavigationControllerDelegate {
    let imagePicker =  UIImagePickerController()
    let alertController = UIAlertController()
    var hasCameraAccess = false
    
    override func viewDidLoad() {
        super.viewDidLoad()
        ProductCategory.sync()
        SceneLocation.sync()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        checkCameraAccess()
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
            self.checkCameraAccess()
        }
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

        let desiredSize = CGSize(width: 2024,height: 2024)
        var photoToLoad:UIImage?
        if let editedImage = info[.editedImage] as? UIImage {
            photoToLoad = resizeImage(image: editedImage, targetSize: desiredSize)
        } else if let originalImage = info[.originalImage] as? UIImage {
            photoToLoad = resizeImage(image: originalImage, targetSize: desiredSize)
        }
        
        picker.dismiss(animated: true, completion: {
            if let photo = photoToLoad {
                self.pickedImage(image:photo)
            }
        })
    }
    
    func pickedImage(image:UIImage) {
        
    }
}
