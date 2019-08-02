//
//  SelectProjectImage.swift
//  Prestige
//
//  Created by Joel Teply on 6/16/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

fileprivate func < <T : Comparable>(lhs: T?, rhs: T?) -> Bool {
  switch (lhs, rhs) {
  case let (l?, r?):
    return l < r
  case (nil, _?):
    return true
  default:
    return false
  }
}

fileprivate func > <T : Comparable>(lhs: T?, rhs: T?) -> Bool {
  switch (lhs, rhs) {
  case let (l?, r?):
    return l > r
  default:
    return rhs < lhs
  }
}


class ProjectImageBeforeAfterCell: UICollectionViewCell {
    @IBOutlet weak var beforeAfterSlider: ImageBeforeAfterSlider!
    
    weak var delegate: ProjectImageCellDelegate?
    
    var image: VisualizerImage? {
        didSet {
            beforeAfterSlider.image = image
        }
    }
    
    @IBAction func selectedImage() {
        if let image = self.image {
            self.delegate?.projectImageSelected(self, image: image)
        }
    }
    
    @IBAction func deleteProjectAction() {
        if let image = self.image {
            self.delegate?.projectImageDeleted(self, image: image)
        }
    }
}

protocol ProjectImageCollectionDelegate : ProjectImageCellDelegate {
    func projectImageScrolled(_ sender: AnyObject, image:VisualizerImage)
}

protocol ScrollDelegate : NSObjectProtocol {
    func projectImageScrolled(indexPath: IndexPath)
}

class ProjectImageCollection: DetailsCollectionView, ProjectImageCellDelegate, UICollectionViewDelegate, UICollectionViewDataSource {
    
    fileprivate let reuseIdentifier = "ProjectImageBeforeAfterCell"
    weak var scrollDelegate: ScrollDelegate?
    
    weak internal var imageDelegate: ProjectImageCollectionDelegate?
    var project:VisualizerProject? {
        didSet {
            
            self.reloadData()
        }
    }
    
    var lastCurrentImage:VisualizerImage? {
        willSet {
            if (newValue != lastCurrentImage) {
                //print("Changed")
                if let newValue = newValue {
                    self.imageDelegate?.projectImageScrolled(self, image:newValue)
                    self.scrollDelegate?.projectImageScrolled(indexPath: IndexPath(item: self.project!.getImageIndex(image: newValue), section: 0))
                }
            }
        }
    }
    
    @objc dynamic var currentImage:VisualizerImage? {
        get {
            
            var visibleRect = CGRect()
            
            visibleRect.origin = self.contentOffset
            visibleRect.size = self.bounds.size
            
            let visiblePoint = CGPoint(x: visibleRect.midX, y: visibleRect.midY)
            
            if let visibleIndexPath: IndexPath = self.indexPathForItem(at: visiblePoint) , project != nil {
                return imageForIndexPath(visibleIndexPath)
            }
            
            return nil
        }
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        
        lastCurrentImage = currentImage
    }
    
    func removeCurrentImage(_ sender: AnyObject) {
        
        if let image = self.currentImage {
            
            image.deleteImage()
            
            if (self.project?.images.count > 0) {
                self.reloadData()
            } else {
                //self.dismissViewControllerAnimated(true, completion: nil)
            }
        }
    }
    
    func numberOfSections(in collectionView: UICollectionView) -> Int {
        // #warning Incomplete implementation, return the number of sections
        return 1
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        // #warning Incomplete implementation, return the number of items
        if !self.project!.images.isEmpty {
            return self.project!.images.count
        }
        let noImageLabel = UILabel(frame: CGRect(x: 0, y: 0, width: self.bounds.size.width, height: self.bounds.size.height))
        noImageLabel.text = "No images to display!"
        noImageLabel.textAlignment = .center
        self.backgroundView = noImageLabel
        return 0
    }
    
    func imageForIndexPath(_ indexPath: IndexPath) -> VisualizerImage {
        let index = (indexPath as NSIndexPath).row
        let image = self.project!.sortedImages[index]
        return image
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: reuseIdentifier, for: indexPath) as! ProjectImageBeforeAfterCell
        
        if !(self.project!.images.isEmpty) {
            let image = imageForIndexPath(indexPath)
            cell.image = image
            cell.delegate = self
        }
        
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAtIndexPath indexPath: IndexPath) -> CGSize {
        //print(self.frame.size.width)
        return CGSize(width: self.frame.size.width, height: self.frame.size.height)
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        
    }
    
    func projectImageSelected(_ sender: AnyObject, image:VisualizerImage) {
        self.imageDelegate?.projectImageSelected(sender, image: image)
    }
    
    func projectImageDeleted(_ sender: AnyObject, image:VisualizerImage) {
        self.imageDelegate?.projectImageDeleted(sender, image: image)
    }
}
