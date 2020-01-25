//
//  SwatchSelectorController.swift
//  DesignUp
//
//  Created by Joel Teply on 1/23/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import UIKit
import RealmSwift

class SwatchCell: UICollectionViewCell {
    @IBOutlet weak var backgroundImage: UIImageView!
    
    func selected(_ select: Bool, animated:Bool) {
        self.layer.borderWidth = select ? 2.0 : 0.0
        self.layer.borderColor = UIColor.cambrianBlue.cgColor
    }
}

protocol PaintSelectionDelegate: class {
    func paintSelected(_ paint: BrandItem)
}

class SwatchSelectorController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout {
    
    @IBOutlet weak var swatchScroller: UICollectionView!
    
    open weak var delegate: PaintSelectionDelegate?
    
    private var selectedCell:SwatchCell? {
        willSet {
            if let cell = selectedCell {
                cell.selected(false, animated: false)
            }
        }
        didSet {
            selectedCell?.selected(true, animated: false)
        }
    }
    
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
    
    private var paintBrands:[BrandCategory]?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        if let parent = DataSource.current.paintsRealm.objects(BrandCategory.self).filter({$0.parentCategory == nil}).first {
            self.paintBrands = Array(parent.subCategories)
        }
    }
    
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        self.refreshItems()
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
        } else if let brands = self.paintBrands {
            //print("Listing top level categories")
            items = brands
        }
    }
    
    func reloadSwatches() {
        self.selectedCell = nil
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
        let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "SwatchCell", for: indexPath) as! SwatchCell
        
        let item = self.items[indexPath.row]
        if let url = item.thumbnailUrl {
            cell.backgroundColor = UIColor.clear
            cell.backgroundImage.image = UIImage(contentsOfFile: url.path)
        } else {
            cell.backgroundImage.image = nil
            cell.backgroundColor = item.color
        }
        
        return cell
    }
    
    var maxItemSize:CGFloat = 0
    let spacing:CGFloat = 5
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {

        if let layout = collectionViewLayout as? UICollectionViewFlowLayout {
        
            if (collectionView.frame.size.height > 0) {
                maxItemSize = collectionView.frame.size.height - layout.sectionInset.top - layout.sectionInset.bottom
            }
            
            let item = self.items[indexPath.row]
            
            if let _ = item as? BrandCategory, self.items.count < 50 {
                return CGSize(width: maxItemSize - spacing, height: maxItemSize - spacing)
            }
            
            return CGSize(width: maxItemSize / 4 - spacing, height: maxItemSize / 4 - spacing)
            
        }

        return CGSize.zero
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard let cell = collectionView.cellForItem(at: indexPath) as? SwatchCell else {
            fatalError("cannot find SwatchCell")
        }
        
        self.selectedCell = cell
        let item = items[indexPath.row]
        
        if let category = item as? BrandCategory {
            self.category = category
        } else if let paint = item as? BrandItem {
            self.delegate?.paintSelected(paint)
        }
    }
}
