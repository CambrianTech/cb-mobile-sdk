//
//  BrandTypeSelectorViewController.swift
//  HomeHarmony
//
//  Created by Jaremy Longley on 3/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

class AssetTypeSelectionController: UIViewController, UICollectionViewDataSource, UICollectionViewDelegate, UICollectionViewDelegateFlowLayout {
    @IBOutlet weak var assetTypeCollection: UICollectionView!
    @IBOutlet weak var searchButton: RoundButton!
    
    weak internal var delegate: BottomSheetDelegate?
    
    var selectedType: BrandCategory?
    var selectedIndex: IndexPath? {
        didSet {
            self.assetTypeCollection.reloadData()
        }
    }
    var categories = List<BrandCategory>()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.assetTypeCollection.delegate = self
        self.assetTypeCollection.dataSource = self
        // Layout cells
        let layout: UICollectionViewFlowLayout = UICollectionViewFlowLayout()
        layout.minimumInteritemSpacing = 0
        layout.minimumLineSpacing = 0
        assetTypeCollection!.collectionViewLayout = layout
        self.setupAssetTypeCollection()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        // Appending the base categories to the bottom sheet
        if let topCategories = DataController.sharedInstance.brandContext?
            .objects(BrandCategory.self)
            .filter({$0.parentCategory == nil && !$0.isEmpty()}) {
            for category in topCategories {
                print(category.name)
                self.categories.append(category)
            }
        }
        // apply gray border to right side of brandTypeSelectorContainer
        let rightBorder = CALayer()
        rightBorder.backgroundColor = UIColor.gray.cgColor
        rightBorder.frame = CGRect(x: self.view.frame.maxX, y: self.view.frame.minY, width: 1.0, height: self.view.frame.height)
        self.view.layer.addSublayer(rightBorder)
    }
    
    func setupAssetTypeCollection() {
        self.assetTypeCollection.frame = CGRect(x: 0, y: 0, width: (self.view.frame.width) , height: self.view.frame.height)
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return self.categories.count
    }

    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "AssetTypeCell", for: indexPath) as? AssetTypeCell else {
            fatalError("Could not find AssetTypeCell")
        }
        cell.type = self.categories[indexPath.item]
        print(self.categories[indexPath.item].name)
        return cell
    }

    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if self.selectedType != nil && self.selectedIndex != indexPath {
            // shrink the previously selected
            let prevSelected = collectionView.cellForItem(at: self.selectedIndex!) as? AssetTypeCell
            prevSelected?.hasBeenSelected = false
        }
        let cell = collectionView.cellForItem(at: indexPath) as? AssetTypeCell
        cell?.hasBeenSelected = true
        self.selectedType = cell?.type
        self.selectedIndex = indexPath
        self.delegate?.typeSelected(self, category: self.selectedType!)
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {
        let width = (collectionView.frame.width / CGFloat(self.categories.count))
        let height = collectionView.frame.height
        return CGSize(width: width, height: height)
    }
    
    @IBAction func searchPressed(_ sender: Any) {
        self.delegate?.searchPressed(self)
    }
}

class AssetTypeCell: UICollectionViewCell {
    @IBOutlet weak var assetTypeLabel: UILabel!
    @IBOutlet weak var underline: UIView!
    
    var type: BrandCategory? {
        didSet {
            self.assetTypeLabel.text = type?.name.uppercased()
        }
    }
    var hasBeenSelected = false {
        didSet {
            self.underline.backgroundColor = (hasBeenSelected ? cambrianBlue : UIColor.clear)
            self.assetTypeLabel.textColor = (hasBeenSelected ? cambrianBlue : UIColor.black)
            self.assetTypeLabel.font = (hasBeenSelected ? UIFont.systemFont(ofSize: 16.0, weight: .bold) : UIFont.systemFont(ofSize: 15.0, weight: .bold))
        }
    }
    
    override func layoutSubviews() {
        self.underline.isHidden = false
    }
}


















