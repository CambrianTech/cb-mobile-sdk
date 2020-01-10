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
    
    static let maxDataAgeDays = 30
    static let webSource = "https://shawfloors.com/api"
    static let categoryJsonPath = "datasource.json"
    static let ppiJsonPath = "ppi-data.json"
    static let productGroup = "shawfloors"
    
    static let cambrianWebURL =  "https://mobile.cambrianar.com"
    //static let cambrianWebURL =  "http://10.0.1.61:3000"
    
    static let brandInfoUrl =  URL(string: "\(cambrianWebURL)/brand-info")!
    static let visualizerUrl = URL(string: cambrianWebURL)!
    static let sceneBaseUrl = URL(string: "\(cambrianWebURL)/assets/scenes")!
    static let sceneDataUrl = URL(string: "\(cambrianWebURL)/assets/scenes/scenes.json")!
    static let productDetailsUrl = URL(string: "\(cambrianWebURL)/product-details")!
    
    static let baseImagePath = "https://shawfloors.scene7.com/is/image";
    static let imageSize = 320
    static let pageSize = 1000
    
    let realm:Realm
    
    var ppiData: Dictionary<String, Int> = Dictionary<String, Int>()
    
    private init() {
        
        do {
            //not necessary, just convenient:
            let copied = try FileManager.default.copyBundleFileToUserDocuments(forResource: "default", ofType: "realm")
            if (copied) {
                print("Copied realm database to default location")
            }
        }
        catch { }
        //must run:
        realm = try! Realm()
    }
    
    private func readPPIJSON() {
        if let path = Bundle.main.path(forResource: DataSource.ppiJsonPath, ofType: nil) {
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
    
    func getLocalImagePath(_ remotePath:URL) -> String? {
        let key = SDWebImageManager.shared.cacheKey(for: remotePath)
        let path = SDImageCache.shared.cachePath(forKey: key)
        return path
    }
    
    func getCachedImages(remotePaths:[URL], progress: @escaping (Int, Int) -> Void, completion: @escaping (Bool) -> Void) {
        
        var toLoad = 0
        var totalBytes: [Int] = []
        var receivedBytes: [Int] = []
        
        for remotePath in remotePaths {
            
            if let localPath = getLocalImagePath(remotePath) {
                if !FileManager.default.fileExists(atPath: localPath) {
                    toLoad += 1
                    let index = receivedBytes.count
                    totalBytes.append(0)
                    receivedBytes.append(0)
                    SDWebImageManager.shared.loadImage(with: remotePath, options: [],
                                                         progress: { (receivedSize, expectedSize, targetURL) in
                                                            if (expectedSize > 0) {
                                                                totalBytes[index] = max(expectedSize, receivedSize)
                                                                receivedBytes[index] = receivedSize
                                                                let totalBytesReceived = receivedBytes.reduce(0, +)
                                                                let totalBytesAvailable = totalBytes.reduce(0, +)
                                                                progress(totalBytesReceived, totalBytesAvailable)
                                                            }
                    },
                                                         completed: { (image, data, error, cacheType, completed, url) in
                                                            toLoad -= 1
                                                            if error != nil || !completed {
                                                                completion(false)
                                                            }
                                                            else if (toLoad == 0) {
                                                                completion(true)
                                                            }
                    })
                }
            }
        }
        
        if (toLoad == 0) {
            completion(true)
        }
    }
}
