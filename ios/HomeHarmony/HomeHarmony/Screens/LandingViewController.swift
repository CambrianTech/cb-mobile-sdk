//
//  LandingViewController.swift
//  HomeDecoratorApp
//
//  Created by Joel Teply on 11/16/15.
//  Copyright © 2015 Joel Teply. All rights reserved.
//

import UIKit
import Photos

protocol ProjectReaderDelegate: NSObjectProtocol {
    func updateProjectPath(_ path: String);
}

class LandingViewController: UIViewController, ProjectReaderDelegate {
    
    @IBOutlet var vizProjectButton:UIButton!
    
    fileprivate var isSample = false
    fileprivate var newProject = false
    fileprivate var storedImage:UIImage!
    fileprivate var sharedProjectPath:String!
    
    @IBOutlet weak var startVisualizerButton: LandingPageButton!
    @IBOutlet weak var colorFinderButton: MainButton!
    @IBOutlet weak var imageGalleryButton: MainButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.startVisualizerButton.isEnabled = false
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        ImageManager.sharedInstance.askForCameraAccess(self, handler: { (status) -> Void in
            // ask for camera access first
            if (status == .authorized) {
                self.startVisualizerButton.isEnabled = true
            } else {
                //ask them to do it manually
                ImageManager.sharedInstance.noCameraAccessAlert(self, showSettings: true)
            }
        })
        
        vizProjectButton.isHidden = VisualizerProject.currentProject.isEmpty && VisualizerProject.latestProjects().count <= 1
        
        
        self.navigationController?.navigationBar.backgroundColor = UIColor.clear
        self.navigationController?.navigationBar.tintColor = UIColor.white
        self.navigationController?.useTransparentNavigationBar()
    }
    
    @IBAction func vizGalleryButtonClicked(_ sender: AnyObject) {
        self.newProject = true
        ImageManager.sharedInstance.pickerCallback = { (image) -> Void in
            if let img = image {
                self.storedImage = img
                self.performSegue(withIdentifier: "showVisualizer", sender: self)
            }
        }
        ImageManager.sharedInstance.showPhotoLibrary(self, type:.photoLibrary)
    }
    
    @IBAction func searchButtonClicked(_ sender: Any) {
        let searchVC: SearchViewController? = UIStoryboard(name: "Search", bundle: nil).instantiateInitialViewController() as! SearchViewController
        searchVC?.backButtonText = "Home"
        self.navigationController?.pushViewController(searchVC!, animated: true)
    }
    
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let vc = segue.destination as? ARViewController {
            vc.backButtonText = "Home"
            if let storedImage = self.storedImage {
                vc.setRawImage(storedImage)
            }
            else if let sharedProjectPath = self.sharedProjectPath {
                vc.incomingProject(sharedProjectPath)
                vc.displayEditable()
            }
        }
        
        self.storedImage = nil
        self.isSample = false
        self.sharedProjectPath = nil
    }
    
    func updateProjectPath(_ path: String) {
        self.sharedProjectPath = path
    }
}



