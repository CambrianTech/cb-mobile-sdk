//
//  SwatchSelectorController.swift
//  DesignUp
//
//  Created by Joel Teply on 1/23/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import UIKit
import RealmSwift

class SwatchSelectorController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout {
    
    @IBOutlet weak var swatchScroller: UICollectionView!
    
    private var product: BrandItem? {
        didSet {
            self.refreshItems()
        }
    }
    
    private var _category: BrandCategory?
    var category: BrandCategory? {
        get {
            if let product = product {
                return product.parentCategory
            }
            return _category
        }
        set {
            _category = newValue
            self.refreshItems()
        }
    }
    
    private var topLevelCategories:[BrandCategory]?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        if let parent = DataSource.current.paintsRealm.objects(BrandCategory.self).filter({$0.parentCategory == nil}).first {
            self.topLevelCategories = Array(parent.subCategories)
        }
        
        self.refreshItems()
        self.reloadSwatches()
    }
    
    var items:[SwatchObject] = [] {
        didSet {
            reloadSwatches()
        }
    }
    
    func refreshItems() {
        if let parent = self.category {
            //print("Listing \(parent.products.count > 0 ? "products" : "categories") for category \(parent.name)")
            if (parent.items.count > 0) {
                items = Array(parent.items)
            } else {
                items = Array(parent.subCategories)
            }
        } else if let categories = self.topLevelCategories {
            //print("Listing top level categories")
            items = categories
        }
    }
    
    func reloadSwatches() {
        self.swatchScroller.refreshLayout()
        self.swatchScroller.reloadData()
        self.swatchScroller.performBatchUpdates(nil, completion: {
            (result) in
             if let product = self.product, let index = self.items.firstIndex(of: product) {
                self.swatchScroller.scrollToItem(at: IndexPath(row: index, section: 0), at: .centeredHorizontally, animated: true)
             } else if let category = self.category, let index = self.items.firstIndex(of: category) {
                self.swatchScroller.scrollToItem(at: IndexPath(row: index, section: 0), at: .centeredHorizontally, animated: true)
             }
        })
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return self.items.count
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "SwatchCell", for: indexPath)
        cell.backgroundColor = self.items[indexPath.row].color
        return cell
    }
}
