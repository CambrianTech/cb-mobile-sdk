//
//  BrandItem_Extended.swift
//  HomeHarmony
//
//  Created by Joel Teply on 11/7/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift
import FirebaseStorage
import Zip

internal protocol ItemSelectionDelegate : NSObjectProtocol {
    func itemSelected(_ sender: AnyObject, item:BrandItem)
    func categoryScrolledTo(_ sender: AnyObject, category:BrandCategory, indexPath:IndexPath)
}

extension BrandItem: Drawable {
    
    var hasThumbnail:Bool {
        get {
            return self.type != .paint
        }
    }
    
    static let metaDataItemIDField = "materialID"
    
    func needsDarkText() -> Bool {
        let calculation = ((self.red * 299) + (self.green * 587) + (self.blue * 114)) / 1000
        if calculation < 125 {
            return false
        } else {
            return true
        }
    }

    
    func getRootCategory() -> BrandCategory {
        var category = self.parentCategory
        var prevCategory = self.parentCategory
        while(category?.parentCategory != nil) {
            prevCategory = category!
            category = category?.parentCategory
        }
        // The while loop will return "Paint" and "Flooring", so get the next category down
        // This should be used if 'Paint' and 'Flooring' are included in assets db
        category = prevCategory
        return (category)!
    }
    
    class func itemForID(_ id: String) -> BrandItem? {
        if id.characters.count > 0 {
            return DataController.sharedInstance.brandContext?.object(ofType: BrandItem.self, forPrimaryKey: id)
        }
        return nil
    }
    
    class func randomItem(_ itemType:CBAssetType? = nil) -> BrandItem? {
        if let realm = DataController.sharedInstance.brandContext {
            
            let allItems = realm.objects(BrandItem.self)
            var items = allItems
            
            if let itemType = itemType {
                let predicate = NSPredicate(format: "type == %d",
                                            itemType.rawValue)
                items =  allItems.filter(predicate)
            }
            
            if (items.count > 0) {
                return items[items.count.random]
            }
        }
        return nil
    }
    
    class func randomBest() -> BrandItem? {
        if let paint = BrandItem.randomItem(.paint) {
            return paint
        } else if let floor = BrandItem.randomItem(.floor) {
            return floor
        } else if let model = BrandItem.randomItem(.model) {
            return model
        }
        return nil
    }
    
    var text: String? {
        return self.name
    }
    
    func isFloor() -> Bool {
        return self.type == .floor
    }
    
    func isPaint() -> Bool {
        return self.type == .paint
    }
    
    var color:UIColor {
        get {
            return UIColor(red: CGFloat(self.red) / 255.0,
                           green: CGFloat(self.green) / 255.0,
                           blue: CGFloat(self.blue) / 255.0,
                           alpha: CGFloat(self.opacity) / 255.0)
        }
        set {
            var fRed : CGFloat = 0
            var fGreen : CGFloat = 0
            var fBlue : CGFloat = 0
            var fAlpha: CGFloat = 0
            if !newValue.getRed(&fRed, green: &fGreen, blue: &fBlue, alpha: &fAlpha) {
                return
            }
            
            self.red = Int(fRed * 255.0)
            self.green = Int(fGreen * 255.0)
            self.blue = Int(fBlue * 255.0)
            self.opacity = Int(fAlpha * 255.0)
            
            var fHue : CGFloat = 0
            var fSat : CGFloat = 0
            var fBright : CGFloat = 0
            
            if !newValue.getHue(&fHue, saturation: &fSat, brightness: &fBright, alpha: &fAlpha) {
                return
            }
            
//            self.hue = Int(fHue * 360.0)
//            self.saturation = Int(fSat * 255.0)
//            self.brightness = Int(fBright * 255.0)
        }
    }
    
    class func getCloseColors(_ color: UIColor, range:Int!=20) -> Results<BrandItem> {
        
        let rgba = colorRGBA(color)
        let red = Int(rgba[0] * 255)
        let green = Int(rgba[1] * 255)
        let blue = Int(rgba[2] * 255)
        
        let context = DataController.sharedInstance.brandContext
        
        let predicate = NSPredicate(format: "(red BETWEEN {%i, %i}) AND (green BETWEEN {%i, %i}) AND (blue BETWEEN {%i, %i})",
                                    red - range, red + range, green - range, green + range, blue - range, blue + range)
        let near = context?.objects(BrandItem.self).filter(predicate)
        //print("near colors count: \(near!.count)")
        return near!
    }
    
    class func getClosestMatch(_ color: UIColor, range:Int!=20) -> BrandItem? {
        let closest = getCloseColors(color, range:range)
        var nearest = CGFloat.greatestFiniteMagnitude
        var bestMatch: BrandItem? = nil
        
        for item in closest {
            let distance = distanceBetweenColors(item.color, color2: color)
            if  distance < nearest {
                nearest = distance
                bestMatch = item
            }
        }
        
        return bestMatch
    }
    
    var view: UIView? {
        get {
            let view = UIImageView()
            view.backgroundColor = self.color
            view.contentMode = .scaleAspectFill
            // This is cached
            view.kf.setImage(with: getThumbnailPath())
            
            return view
        }
    }
    
    func gotoStore() {
        UIApplication.shared.openURL(self.storeURL)
    }
    
    @objc dynamic var storeURL:URL {
        return getStoreURL(self.storeLink)
    }
    
    func getDownloadAssetPath() -> URL {
        return DataController.getWriteDirectory().appendingPathComponent("dbassets").appendingPathComponent(self.assetPath)
    }
    
    func getBundleAssetPath() -> URL {
        return URL(fileURLWithPath: Bundle.main.resourcePath!).appendingPathComponent("dbassets").appendingPathComponent(self.assetPath)
    }
    
    func getBestAssetPath() -> URL {
        
        let downloadPath = getDownloadAssetPath()
        let bundlePath = getBundleAssetPath()

        if FileManager.default.fileExists(atPath: bundlePath.path) {
            return bundlePath
        } else {
            return downloadPath
        }
    }

    func getDiffusePath() -> URL? {
        if self.type == .floor {
            var url = getBestAssetPath()
            url = url.appendingPathComponent("Base_Color.jpg")
            return url
        }
        return nil
    }
    
    func getThumbnailPath() -> URL? {
        if self.hasThumbnail {
            let url = getBestAssetPath()
            if url == self.getBundleAssetPath() {
                return url.appendingPathComponent("Thumbnail.jpg")
            } else {
                let util = AWSAssetUtility(self)
                return util.downloadPath().appendingPathComponent("Thumbnail.jpg")
            }
        }
        return nil
    }
    
    func hasAllAssets(_ files:[String]) -> Bool {
        
        switch type {
        case .floor:
                return (files.count >= 3)
        case .model:
                return files.contains(where: {($0 as NSString).pathExtension == "scn"; })
            default:
                return true;
        }
    }
    
    func downloadAssets(completed: @escaping (_ success:Bool) -> Void) {
        let apath = getBestAssetPath()

        var files = [String]()
        do {
            files = try FileManager.default.contentsOfDirectory(atPath: apath.path)
        } catch { }

        // Make sure the files don't already exist
        if hasAllAssets(files) {
            self.downloadCompleted(true, completed)
            return
        } else {
            let downloadUtil = AWSAssetUtility(self)
            downloadUtil.downloadAsset { success in
                
                DispatchQueue.main.async {
                     completed(success)
                }
            }
        }
    }
    
    func downloadCompleted(_ success:Bool, _ completed: @escaping (_ success:Bool) -> Void) {
        hideProgress()
        completed(success)
    }
}



