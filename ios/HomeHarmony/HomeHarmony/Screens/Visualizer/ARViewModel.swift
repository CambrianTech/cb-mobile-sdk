//
//  ARViewModel.swift
//  HomeHarmony
//
//  Created by Jaremy Longley on 3/26/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import Realm
import RealmSwift

class ARViewModel: NSObject, AssetDelegate {
    
    var controller: ARViewController?
    let defaults = UserDefaults.standard
    var image = VisualizerImage()
    var noItem = true   // Used to determine if an item is present or not
    var hasCreatedFloor = false
    var hasPainted = false
    var sentItem:BrandItem? {
        didSet {
            self.noItem = false
        }
    }
    var selectedItem:BrandItem?
    var rawImage: UIImage?
    var incomingImagePath: String? = nil {
        didSet {
            self.noItem = false
        }
    }
    var incomingSharedProject: Bool? = false {  // A project that comes from a deep link is set here
        didSet {
            if incomingSharedProject! {
                self.noItem = false
            }
        }
    }
    
    /************************
     *
     *      INITIALIZING
     *
     *///////////////////////
    
    init(_ controller: ARViewController) {
        self.controller = controller
    }
    
    func createScene() {
        if let rawImage = self.rawImage {
            //print("loading image")
            if let scene = CBRemodelingScene(uiImage: rawImage) {
                self.initScene(scene)
                self.controller?.enableLiveView(false)
            }
        } else if let imagePath = self.incomingImagePath {
            //print("loading image from path")
            if let scene = CBRemodelingScene(path: imagePath) {
                self.initScene(scene)
                self.controller?.enableLiveView(false)
            }
        } else {
            self.image = VisualizerImage.getImage(self.controller!.getAugmentedView().scene.sceneID)
            self.controller?.enableLiveView(true)
        }
    }
    
    func initScene(_ scene : CBRemodelingScene) {
        self.controller?.setAugmentedViewScene(scene)
        self.image = VisualizerImage.getImage(scene.sceneID)
    }
    
    func initAssets() {
        if let asset = self.image.assets.first {
            self.assetUpdated(asset)
        }
    }
    
    func getSelectedItem() -> BrandItem {
        return self.selectedItem!
    }
    
    /******************************
     *
     *   CHECKS FOR THE CONTROLLER
     *
     */////////////////////////////
    
    func hasImagePath() -> Bool {
        return (self.incomingImagePath == nil)
    }
    
    func hasSentItem() -> Bool {
        return self.sentItem != nil
    }
    
    func hasRawImage() -> Bool {
        return (self.rawImage == nil)
    }
    
    func isFirstItem() -> Bool {
        return self.noItem
    }
    
    /***********************************
     *
     *     MESSAGE/TUTORIAL HANDLING
     *
     *//////////////////////////////////
    
    func maybeShowItemTutorial(_ item:BrandItem) {
        var tutorial: Tutorial
        if (item.type == .paint) {
            tutorial = .Paint
        } else if (item.type == .floor) {
            tutorial = .Floor
        } else {
            tutorial = .Model
        }
        if !self.hasShownTutorial(tutorial) {
            self.controller?.showTutorial(tutorial)
        }
    }
    
    func hasShownTutorial(_ tutorial: Tutorial) -> Bool {
        let settingsKey = "hasSeen\(tutorial.rawValue)Tutorial"
        return defaults.bool(forKey: settingsKey)
    }
    
    func setHasShownTutorial(_ tutorial: Tutorial) {
        let settingsKey = "hasSeen\(tutorial.rawValue)Tutorial"
        
        defaults.set(true, forKey: settingsKey)
        defaults.synchronize()
    }
    
    func getTutorialMessage(_ tutorial: Tutorial) -> String {
        switch tutorial {
        case .Rotation:
            return "Rotate flooring by twisting with two fingers"
        case .Detail:
            return "Draw or erase areas that require further detail"
        case .Paint:
            return "Tap on a wall to apply a color"
        case .Floor:
            return "Tap on the floor"
        case .Model:
            return "Tap on the floor to place model"
        default:
            return String()
        }
    }
    
    /************************
     *
     *     ITEM HANDLING
     *
     *///////////////////////
    
    func itemSelected(item: BrandItem)  {
        let callback = {
            if self.noItem {
                self.noItem = false
            }
            hideProgress()
            self.maybeShowItemTutorial(item)
            self.controller?.setSelectedItem(item: item)
        }
        
        if item.hasThumbnail == false {
            callback()
        } else {
            item.downloadAssets(completed: { (success) in
                if (success) {
                    callback()
                } else {
                    //show unavailable
                    displayError(message: "Asset is unavailable")
                }
            })
        }
    }
    
    /************************
     *
     *     ASSET HANDLING
     *
     *///////////////////////
    
    func assetUpdated(_ asset: Asset) {
        if let item = asset.item, let selectedAsset = self.controller?.getAugmentedView().scene.selectedAsset {
            if let floor = selectedAsset as? CBRemodelingFloor, item.isFloor() {
                floor.setPath(item.assetPath, scale: item.scale)
            } else if let paint = selectedAsset as? CBRemodelingPaint, item.isPaint() {
                paint.color = item.color
            } else if let furniture = selectedAsset as? CBRemodelingFurniture {
                furniture.setPath(item.assetPath, scale: item.scale)
            }
            
            self.assetSelected(asset)
            selectedAsset.setUserData("ID", value: item.id)
            //            favoritesButton.tintColor = item.isInFavorites ? UIColor.red : UIColor.white
        }
    }
    
    func assetSelected(_ asset: Asset) {
        self.selectedItem = asset.item
        if let selected = self.controller!.augmentedView.scene.assets[asset.assetID] {
            self.controller!.augmentedView.scene.selectedAsset = selected
        }
    }
    
    func existingCBAugmentedAsset(_ asset: Asset) -> CBAugmentedAsset? {
        return self.controller?.augmentedView.scene.assets[asset.assetID]
    }
    
    func newCBAugmentedAsset(_ itemType: CBAssetType, _ assetID: String) -> CBAugmentedAsset? {
        if (!self.controller!.augmentedView.scene.canAppendAsset(itemType)) {
            return nil
        }
        
        switch itemType {
        case .paint:
            return CBRemodelingPaint(assetID: assetID)
        case .floor:
            return CBRemodelingFloor(assetID: assetID)
        case .model:
            return CBRemodelingFurniture(assetID: assetID)
        default:
            return nil
        }
    }
    
    func appendCBAugmentedAsset(_ asset: CBAugmentedAsset) {
        //print("Appending asset with ID \(asset.assetID)")
        self.controller!.augmentedView.scene.appendAsset(asset)
    }
    
    func replaceCBAugmentedAsset(_ cbAsset: CBAugmentedAsset, _ forAsset: Asset) {
        self.controller!.augmentedView.scene.removeAsset(forAsset.assetID)
        self.controller!.augmentedView.scene.appendAsset(cbAsset)
    }
    
    func removeAsset(_ asset: Asset) {
        if let asset = self.controller!.augmentedView.scene.assets[asset.assetID] {
            if self.controller!.augmentedView.scene.removeAsset(asset.assetID) {
                print("Deleted asset with ID \(asset.assetID)")
            } else {
                print("failed to delete")
            }
        }
    }
    
    /**************************
     *
     *     PROJECT HANDLING
     *
     */////////////////////////
    
    func saveCapturedImage() {
        let project = VisualizerProject.currentProject
        let callback = {
            displayProgress("saving project...", progress: 0.0)
            
            project.appendImage(image: self.image)
            let path = self.image.directoryPath!.path
            self.image.markModified()
            self.controller?.saveProject(path)
        }
        project.renameAlert(self.controller!, handler: { (completed) in
            if completed {
                callback()
            }
        })
    }
    
    /************************
     *
     *        UNDO
     *
     *///////////////////////
    
    func historyChanged(_ asset: CBAugmentedAsset, change: CBUndoChange, forward: Bool) {
        switch change {
        case .mask:
            print("something was (drawn/undrawn)")
            if selectedItem!.isFloor() {
                hasCreatedFloor = true
                self.controller?.showTutorial(.Rotation)
            } else if selectedItem!.isPaint() {
                hasPainted = true
                controller?.showTutorial(.Detail)
            }
            break
        case .paintColor:
            if(!forward) {
                controller?.undoResult(asset)
            }
            print("paint color was changed")
            break
        case .paintSheen:
            break
        }
    }
}
