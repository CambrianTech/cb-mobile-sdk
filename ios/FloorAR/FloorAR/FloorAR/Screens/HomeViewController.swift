//
//  HomeViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit

class HomeViewController: UIViewController, UIImagePickerControllerDelegate, UINavigationControllerDelegate {
    
    let imagePicker =  UIImagePickerController()
    let alertController = UIAlertController()
    var photoToLoad:UIImage?

    override func viewDidLoad() {
        super.viewDidLoad()
        ProductCategory.sync()
        SceneLocation.sync()
    }
    
    @IBAction func roomPhotoClicked(_ sender: Any) {
        let optionMenu = UIAlertController(title: nil, message: "Create Scene", preferredStyle: .actionSheet)
            
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
        
        self.present(optionMenu, animated: true, completion: nil)
    }
    
    func visualizeImage(_ isCamera:Bool) {
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

        if let editedImage = info[.editedImage] as? UIImage {
            photoToLoad = editedImage
        } else if let originalImage = info[.originalImage] as? UIImage {
            photoToLoad = originalImage
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
