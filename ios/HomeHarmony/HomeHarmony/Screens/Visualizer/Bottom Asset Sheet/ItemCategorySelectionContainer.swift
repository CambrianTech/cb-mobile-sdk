//
//  AssetSelectorContainer.swift
//  HomeHarmony
//
//  Created by Jaremy Longley on 3/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation

internal protocol AssetCategoryAction : NSObjectProtocol {
    func categorySelected(_ sender: AnyObject, category:BrandCategory)
}

internal protocol DropdownViewControllerDelegate : NSObjectProtocol {
    func brandSelected(_ sender: AnyObject, brand:BrandCategory)
}

class ItemCategorySelectionContainer: UIViewController, UICollectionViewDataSource, UICollectionViewDelegate, UICollectionViewDelegateFlowLayout, AssetCategoryAction, DropdownViewControllerDelegate  {
    
    @IBOutlet weak var assetCategoryContainer: UIView!
    @IBOutlet weak var itemSelectorCollection: UICollectionView!
    var swatchController: AssetCategorySwatchController?
    var enlargedIndex: Int?
    weak internal var delegate: BottomSheetDelegate?
    var selectedType: BrandCategory? {
        didSet {
            // depending on type selected, load that specific view and send the type through
            print(selectedType?.name)
            self.swatchController?.selectedType = selectedType
            // Reload the itemselector if there is a category selected
            if selectedType?.getItemType() == CBAssetType.paint && self.swatchController?.selectedPaintBrand != nil {
                self.selectedCategory = self.swatchController?.selectedPaint
            } else if selectedType?.getItemType() == CBAssetType.floor && self.swatchController?.selectedFloorBrand != nil {
                self.selectedCategory = self.swatchController?.selectedFloor
            } else {
                self.selectedCategory = nil
            }
        }
    }
    
    var selectedCategory: BrandCategory? {
        didSet {
            self.itemSelectorCollection.reloadData()
        }
    }
    
    var selectedIndex: IndexPath?
    var selectedItem: BrandItem? {
        didSet {
            self.delegate?.itemSelected(self, item: selectedItem!)
        }
    }
    var dropdownHeightConstraint = NSLayoutConstraint()
    var dropDownViewIsDisplayed: Bool = false
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.itemSelectorCollection.dataSource = self
        self.itemSelectorCollection.delegate = self
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if let category = self.selectedCategory {
            return category.items.count
        } else {
            return 0
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ItemSelectorCell", for: indexPath) as? ItemSelectorCell else {
            fatalError("cannot find ItemSelectorCell")
        }
        print(indexPath.item)
        cell.item = self.selectedCategory?.items[indexPath.item]
        if cell.item == self.selectedItem {
            cell.setEnlarged(true, animated: false)
            
        } else {
            cell.setEnlarged(false, animated: false)
        }
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if self.selectedItem != nil && self.selectedIndex != indexPath {
            // shrink the cell for the previously selected item
            let prevSelected = collectionView.cellForItem(at: self.selectedIndex!) as? ItemSelectorCell
            prevSelected?.setEnlarged(false, animated: true)
        }
        let cell = collectionView.cellForItem(at: indexPath) as? ItemSelectorCell
        cell?.setEnlarged(true, animated: true)
        self.selectedItem = cell?.item
        self.selectedIndex = indexPath
        
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let vc = segue.destination as? AssetCategorySwatchController {
            self.swatchController = vc
            vc.selectedType = self.selectedType
            vc.delegate = self
        }
    }
    
    func categorySelected(_ sender: AnyObject, category: BrandCategory) {
        // This category is sent from the dynamically loaded category view
        print("selected: \(category.name)")
        self.selectedCategory = category
    }
    
    func dropButtonPressed(_ sender: AnyObject) {

    }
    
    func brandSelected(_ sender: AnyObject, brand: BrandCategory) {
        self.dropButtonPressed(sender)
        if brand.getItemType() == CBAssetType.paint {
             self.swatchController?.selectedPaintBrand = brand
        } else {
             self.swatchController?.selectedFloorBrand = brand
        }
    }
}

// Cell for item selector collection
class ItemSelectorCell: UICollectionViewCell {
    
    @IBOutlet weak var itemImageView: UIImageView!
    @IBOutlet weak var itemTitle: UILabel!
    @IBOutlet weak var storeID: UILabel!
    
    
    var item: BrandItem? {
        didSet {
            self.itemImageView.backgroundColor = item?.color
            self.itemImageView.kf.setImage(with: item?.getThumbnailPath())
            self.itemTitle.textColor = (item!.needsDarkText() ? UIColor.black : UIColor.white)
            self.storeID.textColor = (item!.needsDarkText() ? UIColor.black : UIColor.white)
            self.itemTitle.text = item?.name
            self.storeID.text = item?.storeID
            self.itemTitle.isHidden = true
            self.storeID.isHidden = true
        }
    }
    
    var original = CGAffineTransform.identity
    var hasBeenSelected = false
    
    override func layoutSubviews() {
        super.layoutSubviews()
//        self.layer.cornerRadius = 8
        self.layer.masksToBounds = true
    }
    
    func setEnlarged(_ enlarged:Bool, animated:Bool) {
        
        if enlarged {
            self.itemTitle.isHidden = false
            self.storeID.isHidden = false
        } else {
            self.itemTitle.isHidden = true
            self.storeID.isHidden = true
        }
        
        if self.hasBeenSelected && enlarged {
            return
        }
        
        self.hasBeenSelected = enlarged
        
        let scale:CGFloat = enlarged ? 1.25 : 1.0
        
        if (animated) {
            UIView.animate(withDuration: 0.2, delay: 0.01, options: .curveEaseOut, animations: {
                if enlarged {
                    self.layoutIfNeeded()
                    self.transform = self.transform.scaledBy(x: scale, y: scale)
                } else {
                    self.transform = self.original
                    self.layoutIfNeeded()
                }
            }, completion: { finished in
            })
        } else {
            if enlarged {
                self.layoutIfNeeded()
                self.transform = self.transform.scaledBy(x: scale, y: scale)
                self.superview?.bringSubview(toFront: self)
            } else {
                self.transform = self.original
                self.layoutIfNeeded()
                self.superview?.sendSubview(toBack: self)
            }
        }
    }
}





