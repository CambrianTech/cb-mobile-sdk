//
//  GenericViewController.swift
//  HarmonyApp
//
//  Created by joseph on 6/1/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import Social
import Branch

class GenericViewController: UIViewController {
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.navigationController?.stopUsingTransparentNavigationBar()
        self.navigationController?.navigationBar.titleTextAttributes = textAttributes
        self.navigationController?.navigationBar.barTintColor = appColor
        self.navigationController?.navigationBar.backgroundColor = appColor
    }
    
    @IBAction func shareVoteButtonPressed(_ sender: Any) {
        if let image = VisualizerProject.currentProject.currentImage {
            self.shareProject(image)
        }
    }
    
    func shareProject(_ image: VisualizerImage) {
        //Upload Data to Firebase Storage
        //print("Project path: \(image.project.directoryPath.path)")
        uploadProject(url: image.project.directoryPath, image.project.projectID)
        
        // Create a Branch Universal Object
        let branchUniversalObject: BranchUniversalObject = BranchUniversalObject(canonicalIdentifier: "item/share")
        branchUniversalObject.addMetadataKey("projectName", value: image.project.name)
        branchUniversalObject.addMetadataKey("projectId", value: image.project.projectID)
        //branchUniversalObject.addMetadataKey("downloadURL", value: downloadURL.path)
        
        let linkProperties: BranchLinkProperties = BranchLinkProperties()
        linkProperties.feature = "sharing"
        
        branchUniversalObject.getShortUrl(with: linkProperties) { (url, error) in
            if error == nil {
                //print("Got my branch link to share: %@", url)
                if let vc = SLComposeViewController(forServiceType: SLServiceTypeFacebook) {
                    vc.setInitialText("Vote on my project!")
                    vc.add(image.beforeAfterImage)
                    let link = URL(string: url!)
                    vc.add(link)
                    self.present(vc, animated: true, completion: nil)
                }
                
            } else {
                //print(error)
            }
        }
    }
}
