//
//  HistorySelectionView.swift
//  FloorAR
//
//  Created by Joel Teply on 1/9/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
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

protocol HistorySelectionDelegate: class {
    func historyChanged(category: ProductCategory?, product: Product?)
}

class HistorySelectionView: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout{
    
    @IBOutlet weak var historyCollection: UICollectionView!
    
    open weak var delegate: HistorySelectionDelegate?
    
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
    
    var selectedCategory:ProductCategory? {
        didSet {
            reloadHistory()
        }
    }
    var selectedProduct:Product? {
        didSet {
            reloadHistory()
        }
    }

    override func viewDidLoad() {
        super.viewDidLoad()

        self.historyCollection.contentInsetAdjustmentBehavior = .never
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        return history.count
    }
    
    internal func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
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
            let label = UILabel()
            label.text = self.history[indexPath.row].name
            label.sizeToFit()
            let width = label.frame.size.width + 10
            return CGSize(width: max(min(width, self.view.frame.size.width / CGFloat(history.count)), 50), height: height)
        }
       
       return CGSize.zero
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        //let cell = collectionView.cellForItem(at: indexPath) as? HistorySwatchCell
        let item = self.history[indexPath.row]
        self.selectedCategory = item.category
        self.selectedProduct = item.product
        self.delegate?.historyChanged(category: item.category, product: item.product)
    }
    
    func reloadHistory() {
        if (self.view.frame.size.height == 0 || self.view.isHidden) {
            return
        }
        self.historyCollection.contentOffset = CGPoint.zero
        self.historyCollection.reloadData()
        self.historyCollection.invalidateIntrinsicContentSize()
        //historyCollectionHeight?.constant = self.history.count > 0 ? historyHeight : 0
    }
}
