//
//  SamplesViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 1/1/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import UIKit

class SampleCell: UICollectionViewCell {
    
    @IBOutlet weak var imageView: UIImageView!
    @IBOutlet weak var label: UILabel!
    
    func initialize() {
        self.isOpaque = false
        self.backgroundColor = .clear
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        initialize()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        initialize()
    }
    
    var sample:SceneLocation? {
        didSet {
            label.text = sample?.name
            imageView.sd_setImage(with: sample?.thumbnailUrl)
        }
    }
}

class SamplesViewController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource {
    
    @IBOutlet weak var sampleListing: UICollectionView!
    
    @IBAction func closeClicked(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }
    
    var samples:[SceneLocation] = [] {
        didSet {
            self.sampleListing.reloadData()
        }
    }
    var selectedIndex = 0
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.samples = SceneLocation.all()
        
        //refresh
        SceneLocation.sync { 
            self.samples = SceneLocation.all()
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return samples.count
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "SampleCell", for: indexPath) as? SampleCell else {
            fatalError("cannot find SampleCell")
        }
        
        cell.sample = samples[indexPath.row]
        
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        self.selectedIndex = indexPath.row
        self.performSegue(withIdentifier: "visualize", sender: nil)
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "visualize" {
            if let vc = segue.destination as? PhotoViewController {
                vc.sceneToLoad = samples[selectedIndex]
            }
        }
    }
}
