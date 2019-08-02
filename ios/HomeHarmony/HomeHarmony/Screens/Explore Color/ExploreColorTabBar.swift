//
//  ExploreColorTabBar.swift
//  HarmonyApp
//
//  Created by joseph on 4/14/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift


class ExploreColorTabBar: UICollectionViewController {
    let reuseIdentifier = "ExploreColorMenuCell"
    var delegate:BrandCategorySelectionDelegate?
    
    var selectedCell:ExploreColorMenuCell? {
        willSet {
            selectedCell?.isSelected = false
        }
    }
    
    var selectedCategory: BrandCategory? {
        didSet {
            //print(selectedCategory?.name)
            selectedCell?.isSelected = false
            self.collectionView?.reloadData()
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.collectionView!.allowsMultipleSelection = false
        self.automaticallyAdjustsScrollViewInsets = false
    }
    
    var hasInitialLoaded = false
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
    }
    
    override func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }
    
    override func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if let size = self.selectedCategory?.parentCategory?.subCategories.count {
            return size
        } else {
            return 0
        }
    }
    
    override func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: reuseIdentifier, for: indexPath) as? ExploreColorMenuCell
            else {
                fatalError()
        }
        
        cell.category = self.selectedCategory?.parentCategory?.subCategories[(indexPath as NSIndexPath).row]
        
        if self.selectedCategory == cell.category {
            self.selectedCell = cell
            self.selectedCell?.isSelected = true
            self.delegate?.categorySelected(self, category: (self.selectedCell?.category!)!, indexPath: indexPath)
            
        }
        return cell
    }
    
    override func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard let cell = collectionView.cellForItem(at: indexPath) as? ExploreColorMenuCell
            else {
                fatalError()
        }
        
        self.selectedCategory = cell.category
        self.selectedCell = cell
        if let category = cell.category {
            self.delegate?.categorySelected(self, category: category, indexPath: indexPath)
        }
    }
}
