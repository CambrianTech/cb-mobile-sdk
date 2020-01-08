//
//  CatalogViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import WebKit

class CatalogCell: UICollectionViewCell {
    
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
    
    var selectedProduct:Product? {
        didSet {
            label.text = selectedProduct?.name
            imageView.sd_setImage(with: selectedProduct?.thumbnailUrl)
        }
    }
}

class CatalogViewController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout, ProductSelectionDelegate {

    @IBOutlet weak var navigationHeight: NSLayoutConstraint!
    @IBOutlet weak var categoryListing: UICollectionView!
    @IBOutlet weak var webview: CBWebView!
    
    var selectedProduct: Product?
        
    override func viewDidLoad() {
        super.viewDidLoad()
        self.categoryListing.contentInsetAdjustmentBehavior = .never
        _navHeight = navigationHeight.constant
    
        let request = URLRequest(url: DataSource.brandInfoUrl)
        webview.load(request)
        categoryListing.isHidden = true
    }
    
    @IBAction func closeClicked(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }
    
    func categoryChanged(category: ProductCategory?) {
        self.selectedCategory = category
    }
    
    private var _navHeight:CGFloat = 0
    private var selectedCategory:ProductCategory? {
        didSet {
            categoryListing.isHidden = self.selectedCategory == nil
            categoryListing.reloadData()
            navigationHeight.constant = selectedCategory == nil ? _navHeight : 45
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return selectedCategory?.products.count ?? 0
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "CatalogCell", for: indexPath) as? CatalogCell, let products = selectedCategory?.products else {
            fatalError("cannot find CatalogCell")
        }
        
        cell.selectedProduct = products[indexPath.row]
        
        return cell
    }
    
    var isLoadingDetails = false
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard let products = selectedCategory?.products else {
            fatalError("cannot find CatalogCell")
        }
        if (isLoadingDetails) {
            return
        }
        self.isLoadingDetails = true
        self.selectedProduct = products[indexPath.row]
        self.selectedProduct?.sync {
            self.performSegue(withIdentifier: "show-details", sender: nil)
        }
    }
    
    private var categorySelector:ProductSelectionView?
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "product-navigation" {
            if let categorySelector = segue.destination as? ProductSelectionView {
                categorySelector.delegate = self
                self.categorySelector = categorySelector
            }
        } else if segue.identifier == "show-details" {
            if let details = segue.destination as? ProductDetailsViewController {
                details.product = self.selectedProduct
                self.isLoadingDetails = false
            }
        }
    }

}
