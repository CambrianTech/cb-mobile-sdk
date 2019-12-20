//
//  CatalogViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit

class CatalogProductCell: UICollectionViewCell {
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
    
    private var _selectedColor:ProductColor?
    var selectedProduct:ProductColor? {
        get {
            if let color = _selectedColor {
                return color
            }
            return nil
        }
        set {
            _selectedColor = newValue
        }
    }
}

class CatalogRow: UICollectionViewCell, UICollectionViewDelegate, UICollectionViewDataSource {
    @IBOutlet weak var productLabel: UILabel!
    
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
    
    var selectedProduct:Product? {
        didSet {
            productLabel.text = selectedProduct?.name
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return 0
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "CatalogProductCell", for: indexPath) as? CatalogProductCell else {
            fatalError("cannot find CatalogRow")
        }
        
        productLabel.text = selectedProduct?.name
        
        return cell
    }
}

class CatalogViewController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout, ProductSelectionDelegate {

    @IBOutlet weak var categoryListing: UICollectionView!
    override func viewDidLoad() {
        super.viewDidLoad()
        self.categoryListing.contentInsetAdjustmentBehavior = .never
    }
    
    func categoryChanged(category: ProductCategory) {
        self.selectedCategory = category
    }
    
    private var selectedCategory:ProductCategory? {
        didSet {
            categoryListing.reloadData()
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        print("Has \(selectedCategory?.products.count ?? 0) products")
        return selectedCategory?.products.count ?? 0
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "CatalogRow", for: indexPath) as? CatalogRow, let products = selectedCategory?.products else {
            fatalError("cannot find CatalogRow")
        }
        
        cell.selectedProduct = products[indexPath.row]
        
        return cell
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "product-navigation" {
            if let categorySelector = segue.destination as? ProductSelectionView {
                categorySelector.delegate = self
                categorySelector.shouldShowHistory = false
            }
        }
    }
}
