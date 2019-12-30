//
//  ProductSelectionView.swift
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
    
    init() {
        self.name = "Material"
    }
    
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
        
    func resetProperties() {
        _color = nil
        _product = nil
        _category = nil
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
            self.productImage.sd_setImage(with: newValue?.thumbnailUrl)
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
            self.productImage.sd_setImage(with: newValue?.thumbnailUrl)
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
            self.productImage.sd_setImage(with: newValue?.thumbnailUrl)
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

class ProductSelectionView: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout {
    
    open weak var delegate: ProductSelectionDelegate?
        
    @IBOutlet weak var historyCollection: UICollectionView!
    @IBOutlet weak var historyCollectionHeight: NSLayoutConstraint?
    @IBOutlet weak var swatchScroller: UICollectionView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.topLevelCategories = DataSource.current.topLevelCategories
        self.historyCollection.contentInsetAdjustmentBehavior = .never
        self.swatchScroller.contentInsetAdjustmentBehavior = .never
        self.historyHeight = self.historyCollectionHeight?.constant ?? 0
        self.historyCollectionHeight?.constant = shouldShowHistory ? historyHeight : 0
        
        self.delegate?.productSelectionViewDidLoad(psv:self)
    }
    
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
            self.historyCollectionHeight?.constant = shouldShowHistory ? historyHeight : 0
        }
    }
    
    public var shouldShowProducts = true
    public var shouldShowColors = true
    
    private var history:[HistoryItem] {
        get {
            var _history:[HistoryItem] = [HistoryItem()]
            if let category = self.selectedCategory {
                //TODO:handle all categories
                _history.append(HistoryItem(category: category))
            }
            if let product = self.selectedProduct {
                _history.append(HistoryItem(product: product))
            }
            
            return _history
        }
    }
    
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
            _selectedCategory = newValue
            if let category = newValue {
                self.delegate?.categoryChanged(category: category)
            }
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
            _selectedProduct = newValue
            if let product = newValue {
                if (selectedCategory != product.category) {
                    selectedCategory = product.category
                }
                self.delegate?.productChanged(product: product)
            }
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

    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if (collectionView == self.historyCollection) {
            return historyView(collectionView, numberOfItemsInSection: section)
        } else {
            return swatchView(collectionView, numberOfItemsInSection: section)
        }
    }
    
    func historyView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return self.shouldShowHistory ? history.count : 0
    }
    
    func swatchView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if let parent = self.selectedProduct {
            print("Listing product \(parent.name) colors")
            return shouldShowColors ? parent.colors.count : 0
        } else if let parent = self.selectedCategory {
            print("Listing \(parent.products.count > 0 ? "products" : "categories") for category \(parent.name)")
            if (parent.products.count > 0) {
                return shouldShowProducts ? parent.products.count : 0
            } else {
                return parent.categories.count
            }
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
        
        if let _ = collectionViewLayout as? UICollectionViewFlowLayout {
            let height = collectionView.frame.size.height
            
            if (collectionView == self.historyCollection) {
                let label = UILabel()
                label.text = self.history[indexPath.row].name
                label.sizeToFit()
                return CGSize(width: min(label.frame.size.width + 10, self.view.frame.size.width / CGFloat(history.count)), height: height)
            }
            return CGSize(width: height, height: height)
        }
        
        return CGSize.zero
    }
    
    func swatchView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProductSwatchCell", for: indexPath) as? ProductSwatchCell else {
            fatalError("cannot find ProductSwatchCell")
        }
    
        if let product = self.selectedProduct {
            cell.color = product.colors[indexPath.row]
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
        if (!self.shouldShowHistory) {
            return
        }
        self.historyCollection.contentOffset = CGPoint.zero
        self.historyCollection.reloadData()
        //historyCollectionHeight?.constant = self.history.count > 0 ? historyHeight : 0
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
        
        if let color = cell.color, let product = self.selectedProduct {
            selectedCell = cell
            self.delegate?.productColorChanged(product:product, color: color)
            return
        } else if let product = cell.product {
            product.sync {
                self.selectedProduct = product
                self.reloadHistory()
                self.reloadSwatches()
            }
        } else if let category = cell.category {
            category.sync {
                self.selectedCategory = category
                self.reloadSwatches()
                self.reloadHistory()
            }
        }
        
        selectedCell = nil
    }

}
