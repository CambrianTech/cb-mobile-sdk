//
//  BrandTypeSelectorViewController.swift
//  HomeHarmony
//
//  Created by Jaremy Longley on 3/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

class AssetTypeSelectionController: UIViewController {
    @IBOutlet weak var assetTypeCollection: UICollectionView!
    @IBOutlet weak var searchButton: UIView!
    @IBOutlet weak var paintTypeButton: UIButton!
    @IBOutlet weak var paintSelect: UIView!
    @IBOutlet weak var floorTypeButton: UIButton!
    @IBOutlet weak var floorSelect: UIView!
    
    weak internal var delegate: BottomSheetDelegate?
    var paintType: BrandCategory?
    var floorType: BrandCategory?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        let searchTap = UITapGestureRecognizer(target: self, action: #selector(self.searchPressed))
        self.searchButton.addGestureRecognizer(searchTap)
        self.paintSelect.isHidden = true
        self.paintSelect.backgroundColor = appColor
        self.floorSelect.isHidden = true
        self.floorSelect.backgroundColor = appColor
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        // Appending the base categories to the bottom sheet
        if let topCategories = DataController.sharedInstance.brandContext?
            .objects(BrandCategory.self)
            .filter({$0.parentCategory == nil && !$0.isEmpty()}) {
            // seperate out the top level categories to tie to button presses
            for category in topCategories {
                print(category.name)
                if category.getItemType() == CBAssetType.paint {
                    self.paintType = category
                } else {
                    self.floorType = category
                }
            }
        }
    }
    
    @IBAction func paintTypePressed() {
        selectType(self.paintTypeButton, paintSelect)
        self.delegate?.typeSelected(self, category: self.paintType!)
    }
    
    @IBAction func floorTypePressed() {
        selectType(self.floorTypeButton, floorSelect)
        self.delegate?.typeSelected(self, category: self.floorType!)
    }
    
    @objc func searchPressed() {
        self.delegate?.searchPressed(self)
    }
    
    func selectType(_ button: UIButton, _ underline: UIView) {
        self.floorTypeButton.setTitleColor(UIColor.black, for: .normal)
        self.paintTypeButton.setTitleColor(UIColor.black, for: .normal)
        self.paintSelect.isHidden = true
        self.floorSelect.isHidden = true
        button.setTitleColor(appColor, for: .normal)
        underline.isHidden = false
    }
}














