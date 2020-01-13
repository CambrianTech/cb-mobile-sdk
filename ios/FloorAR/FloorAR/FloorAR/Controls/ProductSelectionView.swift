//
//  ProductSelectionView.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/8/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import RealmSwift

extension UICollectionView{
 func refreshLayout() {
      let oldLayout = collectionViewLayout as! UICollectionViewFlowLayout
      let newLayout = UICollectionViewFlowLayout()
      newLayout.estimatedItemSize = oldLayout.estimatedItemSize
      newLayout.footerReferenceSize = oldLayout.footerReferenceSize
      newLayout.headerReferenceSize = oldLayout.headerReferenceSize
      newLayout.itemSize = oldLayout.itemSize
      newLayout.minimumInteritemSpacing = oldLayout.minimumInteritemSpacing
      newLayout.minimumLineSpacing = oldLayout.minimumLineSpacing
      newLayout.scrollDirection = oldLayout.scrollDirection
      newLayout.sectionFootersPinToVisibleBounds = oldLayout.sectionFootersPinToVisibleBounds
      newLayout.sectionHeadersPinToVisibleBounds = oldLayout.sectionHeadersPinToVisibleBounds
      newLayout.sectionInset = oldLayout.sectionInset
      newLayout.sectionInsetReference = oldLayout.sectionInsetReference
      collectionViewLayout = newLayout
  }
}

class ProductSwatchCell: UICollectionViewCell {
    @IBOutlet weak var productImage: UIImageView!
    @IBOutlet weak var productLabel: UILabel!
        
    func resetProperties() {
        _color = nil
        _product = nil
        _category = nil
    }
    
    private func setThumbnailUrl(url:URL?) {
        if let url = url, url.isFileURL {
            self.productImage.image = UIImage(contentsOfFile: url.path)
        } else {
            self.productImage.sd_setImage(with: url)
        }
        self.setNeedsDisplay()
    }
    
    private var _color: ProductColor?
    var color: ProductColor? {
        get {
            return _color
        }
        set {
            resetProperties()
            _color = newValue
            self.productLabel.text = newValue?.name
            self.setThumbnailUrl(url:newValue?.thumbnailUrl)
        }
    }
    
    private var _product: Product?
    var product: Product? {
        get {
            if let color = _color {
                return color.product
            }
            return _product
        }
        set {
            resetProperties()
            _product = newValue
            self.productLabel.text = newValue?.name
            self.setThumbnailUrl(url:newValue?.thumbnailUrl)
        }
    }
    
    private var _category: ProductCategory?
    var category: ProductCategory? {
        get {
            if let product = _product {
                return product.category
            }
            return _category
        }
        set {
            resetProperties()
            _category = newValue
            self.productLabel.text = newValue?.name
            self.setThumbnailUrl(url:newValue?.thumbnailUrl)
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
    func productSelectionViewDidLoad(psv:ProductSelectionView)
    func categoryChanged(category: ProductCategory?)
    func productChanged(product: Product)
    func productColorChanged(product: Product, color: ProductColor)
}

extension ProductSelectionDelegate {
    func productSelectionViewDidLoad(psv:ProductSelectionView) {}
    func categoryChanged(category: ProductCategory?) {}
    func productChanged(product: Product) {}
    func productColorChanged(product: Product, color: ProductColor) {}
}

class ProductSelectionView: UIViewController, HistorySelectionDelegate, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout {
    
    open weak var delegate: ProductSelectionDelegate?
    
    @IBOutlet weak var swatchScroller: UICollectionView!
    @IBOutlet weak var historyCollectionHeight: NSLayoutConstraint?
    
    private var selectedCell:ProductSwatchCell? {
        willSet {
            if let cell = selectedCell {
                cell.selected(false, animated: false)
            }
        }
        didSet {
            selectedCell?.selected(true, animated: false)
        }
    }
    
    private var historyHeight:CGFloat = 0
    public var shouldShowHistory = true {
        didSet {
            self.historySelector?.view.isHidden = !shouldShowHistory
            self.historyCollectionHeight?.constant = shouldShowHistory ? historyHeight : 0
        }
    }
    
    public var shouldShowProducts = true
    public var shouldShowColors = true
    
    private var topLevelCategories:[ProductCategory]?
    
    private var _selectedCategory:ProductCategory?
    var selectedCategory:ProductCategory? {
        get {
            return _selectedCategory
        }
        set {
            if _selectedCategory == newValue {
                return
            }
            self.historySelector?.selectedCategory = newValue
            _selectedCategory = newValue
            self.delegate?.categoryChanged(category: newValue)
            self.refreshItems()
        }
    }
    
    private var _selectedProduct:Product?
    var selectedProduct:Product? {
        get {
            return _selectedProduct
        }
        set {
            if _selectedProduct == newValue {
                return
            }
            self.historySelector?.selectedProduct = newValue
            _selectedProduct = newValue
            if let product = newValue {
                if (selectedCategory != product.category) {
                    selectedCategory = product.category
                }
                self.delegate?.productChanged(product: product)
            }
            self.refreshItems()
        }
    }
    
    private var _selectedColor:ProductColor?
    var selectedColor:ProductColor? {
        get {
            return _selectedColor
        }
        set {
            if _selectedColor == newValue {
                return
            }
            _selectedColor = newValue
            if let color = newValue {
                if (selectedProduct != color.product) {
                    selectedProduct = color.product
                }
                self.delegate?.productColorChanged(product: color.product, color: color)
            }
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.topLevelCategories = ProductCategory.all()
        
        self.swatchScroller.contentInsetAdjustmentBehavior = .never
        self.historyHeight = self.historyCollectionHeight?.constant ?? 0
        self.historyCollectionHeight?.constant = shouldShowHistory ? historyHeight : 0
        
        self.delegate?.productSelectionViewDidLoad(psv:self)
    }
    
    override func viewDidLayoutSubviews() {
        self.refreshItems()
    }

    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return items.count
    }
    
    var items:[Object] = [] {
        didSet {
            reloadSwatches()
        }
    }
    
    func refreshItems() {
        if let parent = self.selectedProduct {
            //print("Listing product \(parent.name) colors")
            items = shouldShowColors ? Array(parent.colors) : []
        } else if let parent = self.selectedCategory {
            //print("Listing \(parent.products.count > 0 ? "products" : "categories") for category \(parent.name)")
            if (parent.products.count > 0) {
                items = shouldShowProducts ? Array(parent.products) : []
            } else {
                items = Array(parent.categories)
            }
        } else if let categories = self.topLevelCategories {
            //print("Listing top level categories")
            items = categories
        }
    }
    
    internal func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProductSwatchCell", for: indexPath) as? ProductSwatchCell else {
            fatalError("cannot find ProductSwatchCell")
        }
        
        if (indexPath.row < items.count) {
            cell.color = nil
            cell.product = nil
            cell.category = nil
            let item = items[indexPath.row]
            if let color = item as? ProductColor {
                cell.color = color
                if (cell.color == self.selectedColor) {
                    cell.selected(cell.color == self.selectedColor, animated: false)
                    self.selectedCell = cell
                }
            }
            else if let product = item as? Product {
                cell.product = product
            }
            else if let category = item as? ProductCategory {
                cell.category = category
            }
        }

        return cell
    }
    
    var itemSize:CGFloat = 0
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {
           
       if let layout = collectionViewLayout as? UICollectionViewFlowLayout {
            if (collectionView.frame.size.height > 0) {
                itemSize = collectionView.frame.size.height - layout.sectionInset.top - layout.sectionInset.bottom
            }
            return CGSize(width: itemSize, height: itemSize)
       }
       
       return CGSize.zero
    }
    
    func reloadSwatches() {
        self.swatchScroller.refreshLayout()
        self.swatchScroller.reloadData()
        self.swatchScroller.performBatchUpdates(nil, completion: {
            (result) in
            if let color = self.selectedColor, let index = self.items.firstIndex(of: color) {
                self.swatchScroller.scrollToItem(at: IndexPath(row: index, section: 0), at: .centeredHorizontally, animated: true)
            } else if let product = self.selectedProduct, let index = self.items.firstIndex(of: product) {
                self.swatchScroller.scrollToItem(at: IndexPath(row: index, section: 0), at: .centeredHorizontally, animated: true)
            }
        })
    }

    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard let cell = collectionView.cellForItem(at: indexPath) as? ProductSwatchCell else {
            fatalError("cannot find ProductSwatchCell")
        }
        
        if (indexPath.row < items.count) {
            let item = items[indexPath.row]
            
            if let color = item as? ProductColor, let product = self.selectedProduct {
                selectedCell = cell
                self.delegate?.productColorChanged(product:product, color: color)
                return
            }
            else if let product = item as? Product {
                product.sync {
                    self.selectedProduct = product
                }
            }
            else if let category = item as? ProductCategory {
                category.sync {
                    self.selectedCategory = category
                }
            }
        }
        
        selectedCell = nil
    }
    
    private var historySelector:HistorySelectionView?
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "history" {
            if let history = segue.destination as? HistorySelectionView {
                history.delegate = self
                history.view.isHidden = !shouldShowHistory
                self.historySelector = history
            }
        }
    }
    
    func historyChanged(category: ProductCategory?, product: Product?) {
        self.selectedCell = nil
        self.selectedCategory = category
        self.selectedProduct = product
    }
}
