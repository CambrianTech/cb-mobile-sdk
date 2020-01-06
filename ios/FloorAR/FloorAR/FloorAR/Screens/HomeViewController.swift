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
        imagePicker.sourceType = isCamera ? .camera : .photoLibrary
        present(imagePicker, animated: true)
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        
    }
}
