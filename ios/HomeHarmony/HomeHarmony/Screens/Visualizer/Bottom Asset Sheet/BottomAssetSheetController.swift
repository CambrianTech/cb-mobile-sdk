//
//  SelectorBottomSheet.swift
//  HomeHarmony
//
//  Created by Jaremy Longley on 3/1/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation

internal protocol BottomSheetDelegate : SendItemDelegate {
    func typeSelected(_ sender: AnyObject, category:BrandCategory)
    func searchPressed(_ sender: Any)
}

class BottomAssetSheetController: UIViewController, BottomSheetDelegate {
    
    @IBOutlet weak var brandTypeSelectorContainer: UIView!
    @IBOutlet weak var assetSelectorContainer: UIView!
    var assetSelectorVC: ItemCategorySelectionContainer?
    weak internal var delegate: BottomSheetAction?
    var POSITION = 250
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.setupAssetSelector()
        self.view.isHidden = true
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.view.frame.origin.y = self.view.frame.height
        self.view.frame.size.height = CGFloat(POSITION)
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let vc = segue.destination as? AssetTypeSelectionController {
            vc.delegate = self
        }
    }
    
    func setupAssetSelector() {
        let storyboard = self.storyboard
        self.assetSelectorVC = storyboard?.instantiateViewController(withIdentifier: "AssetSelectorController") as? ItemCategorySelectionContainer
        addChildViewController(self.assetSelectorVC!)
        self.assetSelectorContainer.addSubview(self.assetSelectorVC!.view!)
        assetSelectorVC?.delegate = self
    }
    
    func typeSelected(_ sender: AnyObject, category: BrandCategory) {
        print(category.name)
        self.loadAssetCategorySelectorView(type: category)
    }
    
    func itemSelected(_ sender: AnyObject, item:BrandItem) {
        self.delegate?.itemSelected(sender, item: item)
    }
    
    func searchPressed(_ sender: Any) {
        self.delegate?.searchPressed(sender)
    }
    
    func loadAssetCategorySelectorView(type: BrandCategory) {
        self.assetSelectorVC?.selectedType = type
    }
    
    func show(_ show:Bool) {
        self.view.isHidden = false
        var finalPos:CGFloat = 0
        
        if(show) {
            finalPos = self.view.superview!.frame.height - 190
        } else {
            finalPos = self.view.superview!.frame.height
        }
        
        UIView.animate(withDuration: 0.3, delay: 0.0, options: UIViewAnimationOptions(), animations: {
            self.view.frame.origin.y = finalPos
            self.view.layoutSubviews()
        }, completion: { finished in
            self.view.isHidden = !show
        })
        
    }
}





