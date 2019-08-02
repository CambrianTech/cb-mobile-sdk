//
//  ProjectItemsCollectionViewController.swift
//  HarmonyApp
//
//  Created by Jaremy Longley on 10/23/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation

class ProjectItemsCollectionViewController: UICollectionViewController {
    var project: VisualizerProject = VisualizerProject.currentProject
    weak var projectDelegate: ProjectDetailsDelegate?
    
    override func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }
    
    override func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if let image = project.currentImage {
            return image.assets.count
        }
        return 0
    }
    
    override func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProjectItemCell", for: indexPath) as? ProjectItemCell else {
            fatalError("Could not find ProjectItemCell")
        }
        let index = indexPath.item
        cell.item = self.project.currentImage?.assets[index].getItem()
        return cell
    }
    
    override func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        let index = indexPath.item
        let item = self.project.currentImage?.assets[index].getItem()
        self.projectDelegate?.itemSelected(item!)
    }
    
    func projectImageScrolled() {
        self.collectionView?.reloadData()
    }
}

class ProjectItemCell: UICollectionViewCell {
    @IBOutlet var itemView: BorderedImage!
    var item: BrandItem? {
        didSet {
            self.itemView.backgroundColor = item?.color
            
            //Will only set the thumbnail for flooring
            itemView.kf.setImage(with: item?.getThumbnailPath())
        }
    }
}
