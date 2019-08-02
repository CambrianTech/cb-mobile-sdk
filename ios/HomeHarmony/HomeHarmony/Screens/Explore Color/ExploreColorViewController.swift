//
//  ExploreColorViewController.swift
//  Prestige
//
//  Created by Joel Teply on 7/4/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import UIKit
import RealmSwift
import Kingfisher

class ExploreColorViewController: UIViewController, BrandCategorySelectionDelegate, ItemSelectionDelegate {
    
    var menuCollection:ExploreColorTabBar?
    var paintCollection:ExploreColorCollection?
    var selectedColor:BrandItem?
    weak var bottomSheet:BottomColorSheet!
    
    var selectedCategory: BrandCategory? {
        didSet {
            //print(selectedCategory?.name)
            menuCollection?.selectedCategory = selectedCategory
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.automaticallyAdjustsScrollViewInsets = false
    }
    
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        setupBottomSheet()
    }
    
    func setupBottomSheet() {
        let sheetStoryboard = UIStoryboard(name: "BottomColorSheet", bundle: nil)
        bottomSheet = sheetStoryboard.instantiateViewController(withIdentifier: "BottomColorSheet") as! BottomColorSheet
        addChildViewController(bottomSheet)
        self.view.addSubview(bottomSheet.view)
        bottomSheet.view.isHidden = true
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let vc = segue.destination as? ExploreColorTabBar {
            self.menuCollection = vc
            vc.selectedCategory = self.selectedCategory
            vc.delegate = self
        }
        else if let vc = segue.destination as? ExploreColorCollection {
            self.paintCollection = vc
            self.paintCollection?.category = self.menuCollection?.selectedCell?.category
//            self.paintCollection?.category = self.selectedCategory
            vc.delegate = self
        }
    }
    
    // dictates which paint collection view is scrolled to once a new category is selected
    func categorySelected(_ sender: AnyObject, category:BrandCategory, indexPath:IndexPath) {
        //print(category.name)
        self.paintCollection?.category = category
        // scroll to the correct view
        self.paintCollection?.collectionView?.scrollToItem(at: indexPath, at: [], animated: true)
    }
    
    // dictates what view is selected in the label once a new paint collection is scrolled to
    func categoryScrolledTo(_ sender: AnyObject, category:BrandCategory, indexPath:IndexPath) {
        self.menuCollection?.selectedCategory = category
        // scroll to the correct view
        self.menuCollection?.collectionView?.scrollToItem(at: indexPath, at: [], animated: true)
    }
    
    func itemSelected(_ sender: AnyObject, item:BrandItem) {
        self.selectedColor = item
        self.bottomSheet.item = item
        self.bottomSheet.show(true)
    }
}

@IBDesignable
class ExploreColorMenuCell : UICollectionViewCell {
    @IBOutlet weak var textLabel: UILabel!
    @IBOutlet weak var underline: UIView!
    
    var category: BrandCategory? {
        didSet {
            textLabel.text = category?.name
        }
    }
    
    @IBInspectable
    var selectedColor:UIColor? = UIColor.cyan
    
    @IBInspectable
    var unselectedColor:UIColor? = UIColor.white
    
    static var selectedCell:ExploreColorMenuCell?

#if !TARGET_INTERFACE_BUILDER
    override var isSelected: Bool {
        didSet {
            if (isSelected) {
                ExploreColorMenuCell.selectedCell = self
            }
            else if (ExploreColorMenuCell.selectedCell == self) {
                ExploreColorMenuCell.selectedCell = nil
            }
            self.setNeedsLayout();
        }
    }
    
    override func layoutSubviews() {
        super.layoutSubviews();
        
        underline.isHidden = false
        underline.backgroundColor = isSelected ? self.selectedColor : UIColor.clear
        textLabel.textColor = isSelected ? self.selectedColor : self.unselectedColor
    }
#endif
    
}


// Paints cell background color with item color
class ExploreColorCell : UICollectionViewCell {
    var imageView: UIImageView? = nil
    
    func setupImage() {
        imageView = UIImageView(frame: self.contentView.frame)
        imageView?.backgroundColor = UIColor.clear
        self.contentView.addSubview(imageView!)
    }
    
    var item: BrandItem? {
        didSet {
            self.contentView.backgroundColor = item?.color
            
            if let item = item {
                if item.hasThumbnail {
                    self.imageView?.isHidden = false
                    imageView?.kf.setImage(with: item.getThumbnailPath())
                }
            }
        }
    }
}


//class ExploreColorCollectionCell : UICollectionViewCell, DetailsCollectionViewDelegate {
class ExploreColorCollectionCell : UICollectionViewCell, UICollectionViewDataSource, DetailsCollectionViewDelegate {
    @IBOutlet var collectionView: UICollectionView?
    var delegate:ItemSelectionDelegate?
    
    var category: BrandCategory? {
        didSet {
            //print(category?.name)
            self.collectionView?.backgroundColor = UIColor.clear
            self.collectionView?.backgroundView = nil
            self.collectionView?.reloadData()
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        // #warning Incomplete implementation, return the number of items
        if let colors = self.category?.items {
            //print(colors.count)
            return colors.count
        }
        return 0
    }
    
    internal func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let items = self.category?.items, let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ExploreColorCell", for: indexPath) as? ExploreColorCell
            else {
                fatalError()
        }
        
        let index = (indexPath as NSIndexPath).row
        let item = items[index]
        if item.hasThumbnail {
            cell.setupImage()
        }
        cell.item = item
        
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAtIndexPath indexPath: IndexPath) -> CGSize {
        
        let itemsPerLine:CGFloat = 7
        let totalWidth = self.collectionView!.frame.size.width / (itemsPerLine + 0.8)
        let width = totalWidth
        //print(width)
        return CGSize(width: width, height: width)
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if let colors = self.category?.items {
            let paint = colors[(indexPath as NSIndexPath).row]
            self.delegate?.itemSelected(self, item: paint)
        }
    }
    
    // MARK: DetailsCollectionViewDelegate
    func addCollectionViewItem() {}
    func longPressCollectionViewItem(_ indexPath:IndexPath) {}
    func detailsForCollectionViewItem(_ indexPath:IndexPath) {}
    
    func previewForCollectionViewItem(_ indexPath:IndexPath) -> UIView? {
        return nil
    }
}

class ExploreColorCollection: UICollectionViewController {
    
    let reuseIdentifier = "ExploreColorCollectionCell"
    var delegate:ItemSelectionDelegate?
    let colorCollectionView : ExploreColorCollectionCell? = nil
    
    var visibleIndexPath: IndexPath?
    
    var category: BrandCategory? {
        didSet {
            self.collectionView?.reloadData()
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.collectionView?.backgroundColor = UIColor.clear
        self.collectionView?.backgroundView = nil
    }
    
    
    // MARK: UICollectionViewDataSource
    override func numberOfSections(in collectionView: UICollectionView) -> Int {
        return 1
    }
    
    override func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if let categories = self.category?.parentCategory?.subCategories {
            return categories.count
        }
        return 0
    }
    
    
    override func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let categories = self.category?.parentCategory?.subCategories ,let cell = collectionView.dequeueReusableCell(withReuseIdentifier: reuseIdentifier, for: indexPath) as? ExploreColorCollectionCell
            else {
                fatalError()
        }
        
        cell.delegate = self.delegate
        cell.category = categories[(indexPath as NSIndexPath).row]
        //print("Cell CATEGORY : \(cell.category?.name)")
        return cell
    }
    
    // delegate function to get index for the view that is showing
    override func scrollViewDidEndDecelerating(_ scrollView: UIScrollView) {
        var visibleRect = CGRect()
        
        visibleRect.origin = (self.collectionView?.contentOffset)!
        visibleRect.size = (self.collectionView?.bounds.size)!
        
        let visiblePoint = CGPoint(x: visibleRect.midX, y: visibleRect.midY)
        
        let newIndexPath = (self.collectionView?.indexPathForItem(at: visiblePoint)!)
        if self.visibleIndexPath != newIndexPath {
            self.visibleIndexPath = newIndexPath
            
            if let visibleIndexPath = self.visibleIndexPath {
                self.delegate?.categoryScrolledTo(self, category: (self.category?.parentCategory?.subCategories[visibleIndexPath.item])!, indexPath: visibleIndexPath)
            }
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAtIndexPath indexPath: IndexPath) -> CGSize {
        
        let flowLayout = collectionViewLayout as! UICollectionViewFlowLayout
        let totalWidth = self.view.frame.size.width
        let width = totalWidth - flowLayout.minimumInteritemSpacing
        
        let totalHeight = self.view.frame.size.height - flowLayout.sectionInset.left - flowLayout.sectionInset.right
        let height = totalHeight - flowLayout.sectionInset.top - flowLayout.sectionInset.bottom
        //print("WIDTH: \(width)")
        //print("Height: \(height)")
        return CGSize(width: width, height: height)
    }
}
