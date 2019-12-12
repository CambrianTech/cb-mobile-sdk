//
//  FloorCollectionView.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/8/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit

class HistorySwatchCellBG: UIView {
    
    var colors = [
        UIColor(red:0.37, green:0.73, blue:0.97, alpha:1.0),
        UIColor(red:0.43, green:0.79, blue:0.98, alpha:1.0),
        UIColor(red:0.49, green:0.86, blue:0.98, alpha:1.0),
        UIColor(red:0.55, green:0.92, blue:0.98, alpha:1.0),
    ]
    
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
    
    var level: Int = 0 {
        didSet {
            self.setNeedsDisplay()
        }
    }
    
    var isLast = false {
        didSet {
            self.setNeedsDisplay()
        }
    }
    
    override func draw(_ rect: CGRect) {
        
        let fillColor:UIColor = colors[level % colors.count]
        let nextFillColor:UIColor = isLast ? .clear : colors[(level+1) % colors.count]
        
        let offset:CGFloat = 11
        let nextPath = UIBezierPath(rect: CGRect(x: frame.maxX - offset, y: frame.minY, width: offset, height: rect.height))
        nextFillColor.setFill()
        nextPath.fill()
        
        let path = UIBezierPath()
        fillColor.setFill()
        let midY = frame.minY + 0.5 * rect.height
        
        path.move(to: CGPoint(x: frame.minX, y: frame.minY))
        path.addLine(to: CGPoint(x: frame.maxX - offset , y: frame.minY))
        path.addLine(to: CGPoint(x: frame.maxX, y: midY))
        path.addLine(to: CGPoint(x: frame.maxX - offset, y: frame.maxY))
        path.addLine(to: CGPoint(x: frame.minX, y: frame.maxY))
        path.addLine(to: CGPoint(x: frame.minX, y: frame.minY))
        path.lineCapStyle = .round;
        path.fill()
    }
}

class HistorySwatchCell: UICollectionViewCell {
    @IBOutlet weak var label: UILabel!
    @IBOutlet weak var background: HistorySwatchCellBG!
    
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
}

class HistoryItem {
    var category:ProductCategory?
    var product:Product?
    var name:String?
    
    init(category:ProductCategory) {
        self.category = category
        self.name = category.name
    }
    
    init(product:Product) {
        self.product = product
        self.name = product.name
    }
}

class ProductSwatchCell: UICollectionViewCell {
    @IBOutlet weak var productImage: UIImageView!
    @IBOutlet weak var productLabel: UILabel!
    
    let textColor = UIColor(red: 51, green: 51, blue: 51)
    
    func resetProperties() {
        _variant = nil
        _product = nil
        _category = nil
    }
    
    private var _variant: ProductColor?
    var variant: ProductColor? {
        get {
            return _variant
        }
        set {
            resetProperties()
            _variant = newValue
            self.productLabel.text = newValue?.name
            self.productLabel.textColor = textColor
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.sd_setImage(with: newValue?.thumbnailPath)
        }
    }
    
    private var _product: Product?
    var product: Product? {
        get {
            if let variant = _variant {
                return variant.parents[0]
            }
            return _product
        }
        set {
            resetProperties()
            _product = newValue
            self.productLabel.text = newValue?.name
            self.productLabel.textColor = textColor
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.sd_setImage(with: newValue?.thumbnailPath)
        }
    }
    
    private var _category: ProductCategory?
    var category: ProductCategory? {
        get {
            return _category
        }
        set {
            resetProperties()
            _category = newValue
            self.productLabel.text = newValue?.name
            self.productLabel.textColor = textColor
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.sd_setImage(with: newValue?.thumbnailPath)
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

class FloorCollectionView: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout {
    
    open weak var delegate: ProductSelectionDelegate?
        
    @IBOutlet weak var historyCollection: UICollectionView!
    @IBOutlet weak var swatchScroller: UICollectionView!
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
    
    private var history:[HistoryItem] = []
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
        
        self.topLevelCategories = DataSource.current.topLevelCategories
    }

    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if (collectionView == self.historyCollection) {
            return historyView(collectionView, numberOfItemsInSection: section)
        } else {
            return swatchView(collectionView, numberOfItemsInSection: section)
        }
    }
    
    func historyView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return history.count
    }
    
    func swatchView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if let parent = self.selectedProduct {
            print("Listing product \(parent.name) colors")
            return parent.colors.count
        } else if let parent = self.selectedCategory {
            print("Listing \(parent.products.count > 0 ? "products" : "categories") for category \(parent.name)")
            return parent.products.count > 0 ? parent.products.count : parent.categories.count
        } else if let categories = self.topLevelCategories {
            print("Listing top level categories")
            return categories.count
        }
        return 0
    }

    internal func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        if (collectionView == self.historyCollection) {
            return historyView(collectionView, cellForItemAt: indexPath)
        } else {
            return swatchView(collectionView, cellForItemAt: indexPath)
        }
    }
    
    func historyView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "HistorySwatchCell", for: indexPath) as? HistorySwatchCell else {
            fatalError("cannot find HistorySwatchCell")
        }
        let historyItem = self.history[indexPath.row]
        cell.label.text = historyItem.name
        cell.background.level = indexPath.row
        cell.background.isLast = (indexPath.row == self.history.count - 1)
        
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {
        
        if let flowLayout = collectionViewLayout as? UICollectionViewFlowLayout {
            if (collectionView == self.historyCollection) {
                let label = UILabel()
                label.text = self.history[indexPath.row].name
                label.sizeToFit()
                return CGSize(width: min(label.frame.size.width, self.view.frame.size.width / CGFloat(history.count)), height: flowLayout.itemSize.height)
            }
            return flowLayout.itemSize
        }
        
        return CGSize.zero
    }
    
    func swatchView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProductSwatchCell", for: indexPath) as? ProductSwatchCell else {
            fatalError("cannot find ProductSwatchCell")
        }
    
        if let product = self.selectedProduct {
            cell.variant = product.colors[indexPath.row]
        } else if let category = self.selectedCategory {
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
    
    func reloadHistory() {
        print(self.history.count)
        self.historyCollection.contentOffset = CGPoint.zero
        self.historyCollection.reloadData()
    }

    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if (collectionView == self.historyCollection) {
            historyView(collectionView, didSelectItemAt: indexPath)
        } else {
            swatchView(collectionView, didSelectItemAt: indexPath)
        }
    }
    
    func historyView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        //let cell = collectionView.cellForItem(at: indexPath) as? HistorySwatchCell
        
        self.history = Array(self.history.prefix(indexPath.row))
        self.selectedCategory = self.history.last?.category
        self.selectedProduct = self.history.last?.product
        
        self.selectedCell = nil
        
        reloadSwatches()
        reloadHistory()
    }
    
    func swatchView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        guard let cell = collectionView.cellForItem(at: indexPath) as? ProductSwatchCell else {
            fatalError("cannot find ProductSwatchCell")
        }
        
        if let variant = cell.variant, let product = self.selectedProduct {
            selectedCell = cell
            self.delegate?.productColorChanged(product:product, color: variant)
            return
        } else if let product = cell.product {
            self.selectedProduct = product
            self.history.append(HistoryItem(product:product))
        } else if let category = cell.category {
            self.selectedCategory = category
            self.history.append(HistoryItem(category:category))
        }
        
        selectedCell = nil
        reloadSwatches()
        reloadHistory()
    }

}
