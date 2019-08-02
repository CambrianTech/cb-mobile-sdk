//
//  LayersViewController.swift
//  HarmonyApp
//
//  Created by joseph on 4/17/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//
import Foundation
import RealmSwift
import Kingfisher


class AssetsViewController: AssetsModel {
    
    @IBOutlet weak var addAssetButton: RoundButton!
    
    var image: VisualizerImage!
    //var isFavorites:Bool = false
    
    
    func setup(image: VisualizerImage) {
        self.image = image
        self.assets = image.assets
        
        //add a random color as initial asset if there are none on the image
        if (assets.isEmpty) {
            if let item = BrandItem.randomBest() {
                self.append(item)
            }
        }
        initViews()
        refresh()
    }
    
    
    @IBAction func addAssetPressed(_ sender: Any) {
        append()
        refresh()
    }
    
    override func hideButton(_ hide: Bool) {
        if(getTargetName() == "ShawDemo") {
            addAssetButton.isHidden = true
            return
        }
        addAssetButton.isHidden = hide
    }
    
    override func moveButton(_ pos: Int) {
        addAssetButton.frame.origin.x = CGFloat(-pos)
    }
    
    override func canAppend(_ newItem: BrandItem? = nil) -> Bool {
        if (assets.count >= maxAssets) {
            return false
        }
        return true
    }
    
    override func append(_ newItem: BrandItem?=nil) {
        
        //asset for realm
        var asset = Asset(UUID().uuidString)
        
        var needsAppend = false
        if let itemID = newItem?.id, let itemType = newItem?.type {
            asset.itemID = itemID
            if let cbAsset = self.delegate?.newCBAugmentedAsset(itemType, asset.assetID) {
                self.delegate?.appendCBAugmentedAsset(cbAsset)
                needsAppend = true
            } else {
                return
            }
        }
        
        if let emptyItem = assets.lazy.filter({$0.itemID == nil}).last {
            //only one allowed, but select it below
            asset = emptyItem
        } else {
            needsAppend = true
        }
        
        if needsAppend {
            if let realm = image.realm {
                try! realm.write {
                    assets.append(asset)
                }
            } else {
                assets.append(asset)
            }
        }
        
        selectedIndex = assets.count - 1
        refresh()
        
        self.delegate?.assetSelected(asset)
        self.delegate?.assetUpdated(asset)
    }
    
    override func selectedItem(_ item: BrandItem) {
        setAssetItem(item) { (success) in
            
        }
    }
    
    func setAssetItem(_ item: BrandItem, completed: @escaping (_ success:Bool) -> Void) {
        if(selectedIndex < assets.count && selectedIndex >= 0) {
            let existingAsset = assets[selectedIndex]
            let existingCBAsset = self.delegate?.existingCBAugmentedAsset(existingAsset)
            
            
            if (existingCBAsset?.isUtilized == true && item.type == .model) {
                append(item)
                completed(true)
                return
            } else if let existingItem = existingAsset.item, existingItem.type != item.type, existingCBAsset?.isUtilized == false {
                //if not used, and different types, replace with new different kind of asset (if possible)
                if let cbAsset = self.delegate?.newCBAugmentedAsset(item.type, existingAsset.assetID) {
                    self.delegate?.replaceCBAugmentedAsset(cbAsset, assets[selectedIndex])
                } else {
                    completed(false)
                    return //could not create a new one, do nothing
                }
            } else if existingAsset.item?.type != item.type { //no item yet or item is used
                if let cbAsset = self.delegate?.newCBAugmentedAsset(item.type, existingAsset.assetID) {
                    self.delegate?.appendCBAugmentedAsset(cbAsset)
                } else {
                    completed(false)
                    return //could not create a new one, do nothing
                }
            }
            
            
            if let realm = image.realm {
                try! realm.write {
                    assets[selectedIndex].itemID = item.id
                }
            } else {
                assets[selectedIndex].itemID = item.id
            }
            refresh()
            completed(true)
        }
        
        self.delegate?.assetUpdated(assets[selectedIndex])
    }
    
    func removeLayer(index: Int) {
        if assets.count > index {
            // remove from visualizer
            self.delegate?.removeAsset(assets[index])
            
            //remove from realm or memory
            if let realm = image.realm {
                try! realm.write {
                    assets.remove(at: index)
                }
            } else {
                assets.remove(at: index)
            }
            
            //select new item after deletion
            selectedIndex = 0
            assetSelected(index: 0)
        }
        refresh()
    }
    
    func undo(_ asset:CBAugmentedAsset) {
        if let containedID = asset.getUserData("ID"),
            let asset = image.assets.lazy.filter({$0.assetID == asset.assetID}).last,
            let item = BrandItem.itemForID(containedID) {

            if let realm = image.realm {
                try! realm.write {
                    asset.itemID = item.id
                }
            } else {
                asset.itemID = item.id
            }

            if let index = assets.index(of: asset) {
                assetSelected(index: index)
            }
            refresh()
        }
    }
    
    override func assetLongPressed(_ sender: UILongPressGestureRecognizer) {
        if sender.state != UIGestureRecognizerState.began { return }
        let index = sender.view!.tag
        
        removeAsset(index: index)
    }
    
    func removeAsset(_ assetID:String) {
        if let index = assets.index(where: {$0.assetID == assetID }) {
            removeAsset(index: index)
        }
    }
    
    private func removeAsset(index:Int) {
        let alert = UIAlertController(title: "", message: "Delete this item?", preferredStyle: UIAlertControllerStyle.alert)
        alert.addAction(UIAlertAction(title: "Cancel", style: UIAlertActionStyle.cancel, handler: nil))
        alert.addAction(UIAlertAction(title: "Delete", style: UIAlertActionStyle.destructive, handler: { action in
            self.removeLayer(index: index)
            if self.assets.count == 0 {
                if let item = BrandItem.randomBest() {
                    self.append(item)
                }
            }
        }))
        
        present(alert, animated: true, completion: nil)
    }
}
