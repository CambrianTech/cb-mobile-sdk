//
//  SampleCollectionViewController.swift
//  HomeDecoratorApp
//
//  Created by Joel Teply on 11/16/15.
//  Copyright © 2015 Joel Teply. All rights reserved.
//

import UIKit
import Zip

internal protocol SampleCellDelegate : NSObjectProtocol {
    func sampleSelected(_ room:SampleRoom, index: Int)
}

class SampleCollectionViewController: UICollectionViewController, SampleImageDelegate, SampleCellDelegate {
    
    let reuseIdentifier = "SampleTypeCollectionCell"
    let sampleImages = SampleImages()

    override func viewDidLoad() {
        super.viewDidLoad()
        sampleImages.delegate = self
        sampleImages.loadJson()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.navigationController?.stopUsingTransparentNavigationBar()
        self.navigationController?.navigationBar.titleTextAttributes = textAttributes
        self.navigationController?.navigationBar.barTintColor = appColor
        self.navigationController?.navigationBar.backgroundColor = appColor
    }
    
    func imagesLoaded() {
        self.collectionView?.reloadData()
    }
    
    func zipDownloaded() {
        hideProgress()
        performSegue(withIdentifier: "showPainter", sender: self)
    }
    
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let vc = segue.destination as? ARViewController {
            if let imagePath = VisualizerProject.currentProject.currentImage?.directoryPath?.path {
                //print("sample image path: \(imagePath)")
                vc.sendImagePath(imagePath)
            }
        }
    }
    
    override func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }
    
    override func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return sampleImages.rooms.count
    }
    
    override func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: reuseIdentifier, for: indexPath)  as! SampleTypeCollectionViewCell
        
        let room = sampleImages.rooms[(indexPath as NSIndexPath).row]
        cell.configureWithRoom(room)
        cell.delegate = self
        
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAtIndexPath indexPath: IndexPath) -> CGSize {
        let flowLayout = collectionViewLayout as? UICollectionViewFlowLayout
        let defaultSize = flowLayout?.itemSize;
        return CGSize(width: self.view.frame.size.width, height: (defaultSize?.height)!)
    }
    
    func sampleSelected(_ room:SampleRoom, index: Int) {
        displayProgress("Downloading...", progress: 0.05)
        //print("Room : \(room.name)")
        //print("Index: \(index)")
        sampleImages.downloadZip(room: room.name, index: String(index))
    }
}



/*
 *  CELL
 */

class SampleCell : UICollectionViewCell {
    @IBOutlet weak var sampleImageView: UIImageView!
    
    var imageURL: URL? {
        didSet {
            sampleImageView.kf.setImage(with: imageURL)
        }
    }
}

class SampleTypeCollectionViewCell: UICollectionViewCell, UICollectionViewDelegate, UICollectionViewDataSource {
    
    static let reuseIdentifier = "SampleCell"
    
    @IBOutlet weak var headingLabel: UILabel!
    @IBOutlet weak var collectionView: DetailsCollectionView!
    weak internal var delegate: SampleCellDelegate?
    fileprivate var room : SampleRoom!
    
    func configureWithRoom(_ room: SampleRoom) {
        self.room = room
        self.headingLabel.text = room.name
        collectionView.reloadData()
    }
    
    func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return self.room.images.count
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: SampleTypeCollectionViewCell.reuseIdentifier, for: indexPath) as? SampleCell
            else {
                fatalError("Requires sample cell")
        }
        
        cell.imageURL = self.room.images[(indexPath as NSIndexPath).row]
        return cell
    }
    
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
            self.delegate?.sampleSelected(room, index: (indexPath as NSIndexPath).row)
    }
}

