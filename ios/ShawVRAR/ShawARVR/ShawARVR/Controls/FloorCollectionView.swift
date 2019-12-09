//
//  FloorCollectionView.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/8/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit

class ProductSwatchCell: UICollectionViewCell {
    @IBOutlet weak var productImage: UIImageView!
    @IBOutlet weak var productLabel: UILabel!
    
    let textColor = UIColor(red: 51, green: 51, blue: 51)
    
    var variant: ProductColor? {
        didSet {
            self.productLabel.text = variant?.name
            self.productLabel.textColor = textColor
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.sd_setImage(with: variant?.thumbnailPath)
        }
    }
    
    var product: Product? {
        didSet {
            self.productLabel.text = product?.name
            self.productLabel.textColor = textColor
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.sd_setImage(with: product?.thumbnailPath)
        }
    }
    
    var category: ProductCategory? {
        didSet {
            self.productLabel.text = category?.name
            self.productLabel.textColor = textColor
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.sd_setImage(with: category?.thumbnailPath)
        }
    }
    
    func selected(_ select: Bool, animated:Bool) {
        self.layer.borderWidth = select ? 2.0 : 0.0
        self.layer.borderColor = UIColor.cambrianBlue.cgColor
    }
    
    func getColor() -> UIColor {
        return UIColor.cambrianBlue
    }
}

protocol ProductSelectionDelegate: class {
    func productColorChanged(product: Product, color: ProductColor)
}

class FloorCollectionView: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource {
    
    open weak var delegate: ProductSelectionDelegate?
        
    @IBOutlet weak var swatchScroller: UICollectionView!
    private var selectedCell:ProductSwatchCell? {
        willSet {
            if let cell = selectedCell {
                cell.selected(false, animated: false)
            }
        }
    }
    
    private var topLevelCategories:[ProductCategory]?
    
    private var _selectedCategory:ProductCategory?
    private var selectedCategory:ProductCategory? {
        get {
            if let category = _selectedCategory {
                return category
            }
            return nil
        }
        set {
            _selectedProduct = nil
            _selectedCategory = newValue
        }
    }
    
    private var _selectedProduct:Product?
    private var selectedProduct:Product? {
        get {
            if let product = _selectedProduct {
                return product
            }
            return nil
        }
        set {
            _selectedProduct = newValue
        }
    }
        
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.topLevelCategories = DataController.sharedInstance.productContext?.objects(ProductCategory.self).sorted(byKeyPath: "orderIndex", ascending: true).filter({$0.parents.count == 0})
    }

    func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }

    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if let parent = self.selectedProduct {
            return parent.colors.count
        }
        else if let parent = self.selectedCategory {
            return parent.products.count > 0 ? parent.products.count : parent.categories.count
        } else if let categories = self.topLevelCategories {
            return categories.count
        }
        return 0
    }

    internal func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProductSwatchCell", for: indexPath) as? ProductSwatchCell else {
            fatalError("cannot find ProductSwatchCell")
        }
    
        if let product = self.selectedProduct {
            cell.variant = product.colors[indexPath.row]
        }
        else if let category = self.selectedCategory {
            if category.products.count > 0 {
                cell.product = category.products[indexPath.row]
            } else {
                cell.category = category.categories[indexPath.row]
            }
        } else if let categories = self.topLevelCategories {
            cell.category = categories[indexPath.row]
        }

        return cell
    }
    
    func reloadSwatches() {
        self.swatchScroller.contentOffset = CGPoint.zero
        self.swatchScroller.reloadData()
    }

    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
                
        let cell = collectionView.cellForItem(at: indexPath) as? ProductSwatchCell
        selectedCell = cell
        
        if let product = cell?.product {
            if let variant = cell?.variant {
                cell?.selected(true, animated: true)
                self.delegate?.productColorChanged(product:product, color: variant)
            } else {
                self.selectedProduct = product
                reloadSwatches()
            }
        } else if let category = cell?.category {
            self.selectedCategory = category
            reloadSwatches()
        }
    }

}
