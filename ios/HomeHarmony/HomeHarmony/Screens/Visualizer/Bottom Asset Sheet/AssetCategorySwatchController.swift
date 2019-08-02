//
//  AssetCategorySwatchController.swift
//  HomeHarmony
//
//  Created by Jaremy Longley on 3/7/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation

class AssetCategorySwatchController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource, UICollectionViewDelegateFlowLayout, UIGestureRecognizerDelegate, DropdownButtonDelegate {
    
    @IBOutlet weak var dropButton: DropButton!
    @IBOutlet weak var assetCategoryCollection: UICollectionView!
    @IBOutlet weak var categoryCollectionLeadingSpace: NSLayoutConstraint!
    weak internal var delegate: AssetCategoryAction?
    
    var selectedType: BrandCategory? {
        didSet {
            // initially load the first category as the selected Category
            if selectedType != nil {
                self.dropButton.isHidden = false
                if isFloorCategory() {
                    // this will just use the label to make a tab view
                    self.dropButton.isHidden = true
                    self.dropButtonPressed = false
                    self.categoryCollectionLeadingSpace.constant = 8
                    self.selectedFloorBrand = selectedType!.subCategories.first!
                } else {
                    self.dropButton.isHidden = false
                    self.categoryCollectionLeadingSpace.constant = 135
                    if self.selectedPaintBrand == nil {
                        self.selectedPaintBrand = selectedType!.subCategories.first!
                        self.dropButton.update(selectedPaintBrand!.name)
                    } else {
                        self.assetCategoryCollection.reloadData()
                    }
                }
            }
        }
    }
    
    var selectedFloorBrand: BrandCategory? {
        didSet {
            print(selectedFloorBrand?.name)
            self.assetCategoryCollection.reloadData()
        }
    }
    
    var selectedPaintBrand: BrandCategory? {
        didSet {
            print(selectedPaintBrand?.name)
            self.assetCategoryCollection.reloadData()
        }
    }
    
    var selectedPaint: BrandCategory?
    var selectedFloor: BrandCategory?
    var selectedIndex: IndexPath?
    var dropButtonPressed = false
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.assetCategoryCollection.delegate = self
        self.assetCategoryCollection.dataSource = self
        if self.selectedType == nil {
            self.dropButton.isHidden = true
        }
        self.dropButton.delegate = self
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if dropButtonPressed {
            return self.selectedType!.subCategories.count
        } else if selectedType != nil {
            if isFloorCategory() {
                if let brand = self.selectedFloorBrand {
                    return brand.subCategories.count
                }
            } else {
                if let brand = self.selectedPaintBrand {
                    return brand.subCategories.count
                }
            }
        }
        return 0
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "CategorySwatchCell", for: indexPath) as? CategorySwatchCell else {
            fatalError("cannot find CategorySwatchCell")
        }
        if isFloorCategory() {
            cell.category = self.selectedFloorBrand?.subCategories[indexPath.item]
        } else {
            cell.category = self.selectedPaintBrand?.subCategories[indexPath.item]
        }
        
        // If cell is the selected one, select that cell when scrolled to without animating
        cell.selected((cell.category == self.selectedFloor || cell.category == self.selectedPaint), animated: false)
        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        let cell = collectionView.cellForItem(at: indexPath) as? CategorySwatchCell
        cell?.selected(true, animated: true)
        if self.dropButtonPressed {
            if let brand = cell?.category {
                self.selectedPaintBrand = brand
                self.dropButton.update(brand.name)
                self.selectedPaintBrand = checkSingleSubcategory(brand)
                self.dropButtonPressed = false
            }
        } else {
            if (self.selectedFloor != nil || self.selectedPaint != nil) && self.selectedIndex != indexPath {
                // shrink the previously selected
                let prevSelected = collectionView.cellForItem(at: self.selectedIndex!) as? CategorySwatchCell
                prevSelected?.selected(false, animated: true)
            }
            self.selectedIndex = indexPath
            
            var category = (isFloorCategory() ? self.selectedFloorBrand?.subCategories[indexPath.item] : self.selectedPaintBrand?.subCategories[indexPath.item] )
            
            if let category = category {
                let cat = checkSingleSubcategory(category)
                if isFloorCategory() {
                    self.selectedFloor = cat
                } else {
                    self.selectedPaint = cat
                }
                self.delegate?.categorySelected(self, category: cat)
            }
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, layout collectionViewLayout: UICollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> CGSize {
        if dropButtonPressed {
            return CGSize(width: 120.0, height: 40.0)
        }
        return CGSize(width: 100.0, height: 40.0)
    }
    
    func dropButtonPressed(_ sender: AnyObject) {
        print(self.selectedType?.name)
        self.dropButtonPressed = true
        self.selectedPaintBrand = self.selectedType
    }
    
    func checkSingleSubcategory(_ category: BrandCategory) -> BrandCategory {
        if category.subCategories.count == 1 {
            return category.subCategories.first!
        } else {
            return category
        }
    }
    
    func isFloorCategory() -> Bool {
        return self.selectedType!.isFloor()
    }
    
}

class CategorySwatchCell: UICollectionViewCell {
    @IBOutlet weak var categoryLabel: UILabel!
    
    let color = UIColor(red: 51, green: 51, blue: 51)
    var category: BrandCategory? {
        didSet {
            self.categoryLabel.text = category?.name
            self.categoryLabel.textColor = color
            self.categoryLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
        }
    }
    
    var hasBeenSelected = false
    
    override func layoutSubviews() {
        super.layoutSubviews()
        self.layer.cornerRadius = 20
        self.layer.masksToBounds = true
        self.layer.borderWidth = 1.0
        self.layer.borderColor = UIColor(red: 239, green: 239, blue: 239).cgColor
    }
    
    func selected(_ select: Bool, animated:Bool) {
        self.hasBeenSelected = select
        self.backgroundColor = (select ? self.getColor() : UIColor.white)
        self.categoryLabel.textColor = (select ? UIColor.white : color)
    }
    
    func getColor() -> UIColor {
        return (self.category!.isFloor() ? cambrianBlue : self.category!.color)
    }
}
