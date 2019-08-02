//
//  AssetsModel.swift
//  HarmonyApp
//
//  Created by Jaremy Longley on 6/27/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//
import Foundation
import RealmSwift
import Kingfisher

internal protocol AssetDelegate: NSObjectProtocol {
    
    func existingCBAugmentedAsset(_ asset:Asset) -> CBAugmentedAsset?
    
    func newCBAugmentedAsset(_ itemType:CBAssetType, _ assetID:String) -> CBAugmentedAsset?
    func appendCBAugmentedAsset(_ asset:CBAugmentedAsset)
    func replaceCBAugmentedAsset(_ cbAsset: CBAugmentedAsset, _ forAsset:Asset);
    
    func assetSelected(_ asset:Asset)
    
    func removeAsset(_ asset:Asset)
    func assetUpdated(_ asset: Asset)
    func getSelectedItem() -> BrandItem
}

class AssetsModel: UIViewController {
    
    weak internal var delegate: AssetDelegate?
    
    var layerViews = [RoundImageBordered]()
    var selectedIndex: Int = 0
    var maxAssets: Int!
    var size = 55
    var assets = List<Asset>()
    var isSample:Bool = false
    
    func initViews() {
        for i in 0..<maxAssets {
            addLayerView(index: i)
        }
        
        if(assets.count > 0) {
            self.selectedIndex = assets.count-1
        } else { self.selectedIndex = 0 }
        
        sideItemSelected(index: selectedIndex)
    }
    
    func addLayerView(index: Int) {
        let tap = UITapGestureRecognizer(target: self, action: #selector(self.assetPressed(_:)))
        let long = UILongPressGestureRecognizer(target: self, action: #selector(self.assetLongPressed(_:)))
        let layerView = RoundImageBordered(frame: CGRect(x:(-(size*index) - size), y: 0, width: size, height: size))
        layerView.setColor(UIColor.gray)
        layerView.tag = index
        layerView.addGestureRecognizer(tap)
        layerView.addGestureRecognizer(long)
        self.view.addSubview(layerView)
        layerViews.append(layerView)
    }
    
    func refresh() {
        hideButton(false)
        for i in layerViews.indices {
            let layer = layerViews[i]
            if(i < assets.count) {
                layer.isHidden = false
                if let item = assets[i].getItem() {
                    layer.setColor(item.color)
                    if item.hasThumbnail {
                        layer.setTexture(item.getThumbnailPath())
                    }
                }
            } else {
                layer.isHidden = true
            }
            layer.isSelected = (i == selectedIndex)
        }
        if(assets.count >= layerViews.count || isSample) {
            hideButton(true)
        }
        let pos = (assets.count * size) + size
        moveButton(pos)
    }
    
    func hideButton(_ hide: Bool) {
        //override in child
    }
    
    func moveButton(_ pos: Int) {
        // override in child
    }
    
    func canAppend(_ newItem: BrandItem? = nil) -> Bool {
        //override in child
        return false
    }
    
    func append(_ newItem: BrandItem) {
        //override in child
    }
    
    // modeled after assetSelected
    func sideItemSelected(index: Int) {
        //override in child
    }
    
    @objc func assetPressed(_ sender: UITapGestureRecognizer) {
        let index = sender.view!.tag
        assetSelected(index: index)
    }
    
    @objc func assetLongPressed(_ sender: UILongPressGestureRecognizer) {
        
    }
    
    func selectedItem(_ item: BrandItem) {
        if (canAppend(item)) {
            append(item)
        }
    }
    
    func assetSelected(index: Int) {
        if assets.count > index {
            selectedIndex = index
            self.delegate?.assetSelected(assets[index])
            refresh()
        }
    }
}
