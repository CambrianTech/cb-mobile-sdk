//
//  CatalogViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit

class CatalogRow: UICollectionViewCell {
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
    
    private var _selectedCategory:ProductCategory?
    var selectedCategory:ProductCategory? {
        get {
            if let category = _selectedCategory {
                return category
            }
            return nil
        }
        set {
            _selectedCategory = newValue
        }
    }
}

class CatalogViewController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout, ProductSelectionDelegate {

    func categoryChanged(category: ProductCategory) {
        self.selectedCategory = category
    }
    
    private var _selectedCategory:ProductCategory?
    private var selectedCategory:ProductCategory? {
        get {
            if let category = _selectedCategory {
                return category
            }
            return nil
        }
        set {
            _selectedCategory = newValue
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return selectedCategory?.products.count ?? 0
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "CatalogRow", for: indexPath) as? CatalogRow, let categories = selectedCategory?.categories else {
            fatalError("cannot find CatalogRow")
        }
        
        cell.selectedCategory = categories[indexPath.row]
        
        return cell
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
}
