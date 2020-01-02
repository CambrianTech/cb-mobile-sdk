//
//  DataSource.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/12/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import Alamofire
import RealmSwift
import SDWebImage

class DataSource {
    
    let categoryJsonPath = "datasource.json"
    let ppiJsonPath = "ppi-data.json"
    let webSource:String
    let productGroup = "shawfloors"
    
    static private let cambrianWebURL =  URL(string:"https://mobile.cambrianar.com")!
    
    static let brandInfoUrl =  URL(string: "brand-info" , relativeTo: cambrianWebURL)!
    static let visualizerUrl = cambrianWebURL
    static let sceneBaseUrl = URL(string: "assets/scenes" , relativeTo: cambrianWebURL)!
    static let sceneDataUrl = URL(string: "assets/scenes/scenes.json" , relativeTo: cambrianWebURL)!
    static let productDetailsUrl = URL(string: "product-details" , relativeTo: cambrianWebURL)!
    
    static let baseImagePath = "https://shawfloors.scene7.com/is/image";
    static let imageSize = 320
    static let pageSize = 1000
    
    let realm = try! Realm()
    
    var topLevelCategories: [ProductCategory] = []
    var jsonCategories: Dictionary<String, Dictionary<String, Any>> = Dictionary<String, Dictionary<String, Any>>()
    
    var ppiData: Dictionary<String, Int> = Dictionary<String, Int>()
    
    private init() {
        if let path = Bundle.main.path(forResource: categoryJsonPath, ofType: nil) {
            do {
                let data = try Data(contentsOf: URL(fileURLWithPath: path), options: .mappedIfSafe)
                let jsonResult = try JSONSerialization.jsonObject(with: data, options: .mutableLeaves)
                
                guard let parsed = jsonResult as? Dictionary<String, AnyObject> else {
                    fatalError("cannot parse json data")
                }
                
                guard let source = parsed["source"] as? String, let categories = parsed["categories"] as? Array<AnyObject> else {
                    fatalError("no source attribute in json data")
                }
                
                self.webSource = source
                for categoryJson in categories {
                    let category = categoryJson as! Dictionary<String, Any>
                    jsonCategories[category["name"] as! String] = category
                }
                self.topLevelCategories = parseProductCategories(categories)
                return
            }
            catch {
                fatalError("required datasource caused error")
            }
        }
        fatalError("cannot find required datasource")
    }
    
    private func readPPIJSON() {
        if let path = Bundle.main.path(forResource: ppiJsonPath, ofType: nil) {
            do {
                let data = try Data(contentsOf: URL(fileURLWithPath: path), options: .mappedIfSafe)
                let jsonResult = try JSONSerialization.jsonObject(with: data, options: .mutableLeaves)
                
                guard let parsed = jsonResult as? Dictionary<String, AnyObject> else {
                    fatalError("cannot parse json data")
                }
                
                guard let imageJSON = parsed["images"] as? Array<AnyObject> else {
                    fatalError("no images attribute in json data")
                }
                
                parsePPIData(imageJSON)
            }
            catch {
                //not required
            }
        }
    }
    
    private static var _client: DataSource?
    
    static var current: DataSource {
        get {
            if let datasource = DataSource._client {
                return datasource
            }
            let datasource = DataSource()
            DataSource._client = datasource
            return datasource
        }
    }
    
    private func parseProductCategories(_ categoriesJSON:Array<AnyObject>) -> [ProductCategory] {
        var categories: [ProductCategory] = []
        for categoryJson in categoriesJSON {
            let category = ProductCategory(categoryJson as! Dictionary<String, AnyObject>)
            categories.append(category)
            try! realm.write {
                realm.add(category)
            }
        }
        return categories
    }
    
    private func parsePPIData(_ imagesJSON:Array<AnyObject>) {
        
        for json in imagesJSON {
            let imageData = json as! Dictionary<String, AnyObject>
            if let imageName = imageData["name"] as? String, let ppi = imageData["ppi"] as? Int {
                self.ppiData[imageName] = ppi
            }
        }
    }
    
    public class func encodeUrl(_ string:String) -> String {
        return string.addingPercentEncoding(withAllowedCharacters:NSCharacterSet.urlQueryAllowed)!
    }
    
    public class func getThumbnailPath(_ UniqueId:String, imageSize:Int=320) -> String {
        return "\(baseImagePath)/ShawIndustries/\(UniqueId)_MAIN?fit=crop&wid=\(imageSize)&hei=\(imageSize)&fmt=jpg"
    }
    
    public class func getDiffusePath(_ UniqueId:String, ppi:Int=20) -> String {
        return "\(baseImagePath)/ShawIndustriesRender/\(UniqueId)_MAIN?res=\(ppi)&resMode=sharp&scl=1&fmt=jpg"
    }
    
    public class func getLocalImagePath(_ remotePath:URL) -> String? {
        let key = SDWebImageManager.shared.cacheKey(for: remotePath)
        let path = SDImageCache.shared.cachePath(forKey: key)
        return path
    }
    
    public func getImagePPI(_ UniqueId:String) -> Int {
        if (self.ppiData.count == 0) {
            readPPIJSON()
        }
        let imageName = "\(UniqueId)_MAIN"
        if let ppi = self.ppiData[imageName] {
            return ppi
        }
        return 20
    }
}
