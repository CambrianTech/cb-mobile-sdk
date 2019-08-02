//
//  ProjectContainerCollectionViewController.swift
//  HarmonyApp
//
//  Created by Jaremy Longley on 10/20/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation

class ProjectContainerCollectionViewController: UICollectionViewController, ProjectImageCollectionDelegate {
    var appendNewImage = false
    
    weak var projectCollectionScrollDelegate: ScrollDelegate?
    var scrollIndex = 0
    override func viewDidAppear(_ animated: Bool) {
        collectionView?.reloadSections(IndexSet(integer: 0))
    }
    
    var project:VisualizerProject? {
        get {
            return VisualizerProject.currentProject
        }
        set {
            if let project = newValue {
                VisualizerProject.currentProject = project
            }
        }
    }
    
    override func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }
    
    override func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return 1
    }
    
    override func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        print("IndexPath: \(indexPath.item)")
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProjectImageCollectionCell", for: indexPath) as? ProjectImageCollectionCell else {
            fatalError("Couldn't find ProjectImageCollectionCell")
        }
        
        cell.projectCollectionImageDelegate = self
        cell.projectCollectionScrollDelegate = self.projectCollectionScrollDelegate
        cell.scrollIndex = self.scrollIndex
        cell.project = self.project
        
        return cell
    }
    
    func projectImageSelected(_ sender: AnyObject, image:VisualizerImage) {
        VisualizerProject.currentProject = image.project!
        VisualizerProject.currentProject.currentImage = image
        self.appendNewImage = false
        performSegue(withIdentifier: "showPainter", sender: self)
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let vc = segue.destination as? ARViewController {
            vc.backButtonText = "Project"
            if let imagePath = VisualizerProject.currentProject.currentImage?.directoryPath?.path {
                self.navigationController?.navigationBar.backgroundColor = UIColor.clear
                print(imagePath)
                vc.sendImagePath(imagePath)
            }
        }
    }
    
    func projectImageDeleted(_ sender: AnyObject, image:VisualizerImage) {
        self.deleteCurrentImage()
    }
    
    func projectImageScrolled(_ sender: AnyObject, image:VisualizerImage) {
        VisualizerProject.currentProject.currentImage = image
    }
    
    func deleteCurrentImage() {
        guard let project = self.project else {
            return;
        }
        
        confirmAction(self, text: "Are you sure you want to delete this image?") {
            if let image = project.currentImage {
                image.deleteImage()
                
                self.collectionView?.reloadData()
            }
        }
    }
}

class ProjectImageCollectionCell: UICollectionViewCell {
    
    @IBOutlet weak var projectCollection: ProjectImageCollection!
    weak var projectCollectionScrollDelegate: ScrollDelegate?
    weak var projectCollectionImageDelegate: ProjectImageCollectionDelegate?
    var scrollIndex = 0
    
    var project: VisualizerProject? {
        didSet {
            projectCollection.delegate = projectCollection
            projectCollection.imageDelegate = self.projectCollectionImageDelegate
            projectCollection.scrollDelegate = self.projectCollectionScrollDelegate
            projectCollection.dataSource = projectCollection
            projectCollection.project = project
            let scrollPath = IndexPath(item: scrollIndex, section: 0)
            print(scrollIndex)
            print(scrollPath)
            projectCollection.scrollToItem(at: scrollPath, at: .centeredHorizontally, animated: false)
        }
    }
}
