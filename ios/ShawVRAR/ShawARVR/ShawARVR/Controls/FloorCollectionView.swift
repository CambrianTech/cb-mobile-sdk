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
    
    let color = UIColor(red: 51, green: 51, blue: 51)
    var product: Product? {
        didSet {
            self.productLabel.text = product?.name
            self.productLabel.textColor = color
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.sd_setImage(with: product?.thumbnailPath)
        }
    }
    
    var category: ProductCategory? {
        didSet {
            self.productLabel.text = category?.name
            self.productLabel.textColor = color
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

class FloorCollectionView: UICollectionViewController {
    
    var categories:[ProductCategory]?
    var categorySelectedIndex = 0
    
    var baseCategory:ProductCategory?
    
    var selectedCell:ProductSwatchCell?
    
    var selectedCategory:ProductCategory? {
        get {
            if let categories = self.categories, categorySelectedIndex < categories.count {
                return categories[categorySelectedIndex]
            }
            return baseCategory
        }
    }
    
    var selectedProduct: Product?
    //weak open var delegate: ProductSelectionDelegate?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        if let _ = baseCategory {
            
        }
        else {
            self.categories = DataController.sharedInstance.productContext?.objects(ProductCategory.self).sorted(byKeyPath: "orderIndex", ascending: true).filter({$0.parents.count == 0})
        }
        
        //self.collectionView!.register(UICollectionViewCell.self, forCellWithReuseIdentifier: "ProductSwatchCell")
    }

    override func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }

    override func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if let parent = self.selectedCategory {
            return parent.products.count > 0 ? parent.products.count : parent.categories.count
        }
        return 0
    }

    override func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProductSwatchCell", for: indexPath) as? ProductSwatchCell else {
            fatalError("cannot find ProductSwatchCell")
        }
    
        if let category = self.selectedCategory {
            if category.products.count > 0 {
                cell.product = category.products[indexPath.row]
            } else {
                cell.category = category.categories[indexPath.row]
            }
        }

        return cell
    }

    // MARK: UICollectionViewDelegate

    /*
    // Uncomment this method to specify if the specified item should be highlighted during tracking
    override func collectionView(_ collectionView: UICollectionView, shouldHighlightItemAt indexPath: IndexPath) -> Bool {
        return true
    }
    */

    /*
    // Uncomment this method to specify if the specified item should be selected
    override func collectionView(_ collectionView: UICollectionView, shouldSelectItemAt indexPath: IndexPath) -> Bool {
        return true
    }
    */

    /*
    // Uncomment these methods to specify if an action menu should be displayed for the specified item, and react to actions performed on the item
    override func collectionView(_ collectionView: UICollectionView, shouldShowMenuForItemAt indexPath: IndexPath) -> Bool {
        return false
    }

    override func collectionView(_ collectionView: UICollectionView, canPerformAction action: Selector, forItemAt indexPath: IndexPath, withSender sender: Any?) -> Bool {
        return false
    }

    override func collectionView(_ collectionView: UICollectionView, performAction action: Selector, forItemAt indexPath: IndexPath, withSender sender: Any?) {
    
    }
    */

}
