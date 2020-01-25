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
    
    static var maxDataAgeDays = 30
    static var imageSize = 320
    static var pageSize = 1000
    static var baseImagePath = "https://shawfloors.scene7.com/is/image"
    static var webSource = "https://shawfloors.com/api"
    static let categoryJsonPath = "datasource.json"
    static let ppiJsonPath = "ppi-data.json"
    static var productGroup = "shawfloors"
    
    static var cambrianWebURL =  "https://mobile.cambrianar.com"
    
    #if DEBUG //because dangerous to leave on
        //static var cambrianWebURL =  "http://10.0.1.61:3000"
    #endif
    
    static var _configUrl = "/config.json"
    static var configUrl:URL {
        get {
            return URL(string: cambrianWebURL + _configUrl)!
        }
    }
    
    static var _brandInfoUrl = "/brand-info"
    static var brandInfoUrl:URL {
        get {
            return URL(string: cambrianWebURL + _brandInfoUrl)!
        }
    }
    
    static var _visualizerUrl = ""
    static var visualizerUrl:URL {
        get {
            return URL(string: cambrianWebURL + _visualizerUrl)!
        }
    }
    
    static var _sceneBaseUrl = "/assets/scenes"
    static var sceneBaseUrl:URL {
        get {
            return URL(string: cambrianWebURL + _sceneBaseUrl)!
        }
    }
    
    static var _sceneDataUrl = "/assets/scenes/scenes.json"
    static var sceneDataUrl:URL {
        get {
            return URL(string: cambrianWebURL + _sceneDataUrl)!
        }
    }
    
    static var _productDetailsUrl = "/product-details"
    static var productDetailsUrl:URL {
        get {
            return URL(string: cambrianWebURL + _productDetailsUrl)!
        }
    }
    
    let realm:Realm
    let paintsRealm:Realm
    
    var ppiData: Dictionary<String, Int> = Dictionary<String, Int>()
    
    private init() {
        
        try! FileManager.default.copyBundleFileToUserDocuments(forResource: "default", ofType: "realm")
        realm = try! Realm()
        
        let path = try! FileManager.default.copyBundleFileToUserDocuments(forResource: "paints", ofType: "realm")
        var paintsConfig = Realm.Configuration()
        paintsConfig.readOnly = true
        paintsConfig.fileURL = path
        paintsRealm = try! Realm(configuration: paintsConfig)
        
        #if DEBUG //because dangerous to leave on
        //AppUpdater.testUpdate = true
        #endif
        
        parseLocalConfig()
        getRemoteConfig()
    }
    
    private func parseLocalConfig() {
        let defaults = UserDefaults.standard
        if let value = defaults.string(forKey: "cambrianWebURL") {
            DataSource.cambrianWebURL = value
        }
        if let value = defaults.string(forKey: "configUrl") {
            DataSource._configUrl = value
        }
        if let value = defaults.string(forKey: "webSource") {
            DataSource.webSource = value
        }
        if let value = defaults.string(forKey: "baseImagePath") {
            DataSource.baseImagePath = value
        }
        if let value = defaults.string(forKey: "productGroup") {
            DataSource.productGroup = value
        }
        if let obj = defaults.object(forKey: "adsEnabled"), let value = obj as? Bool {
            adsEnabled = value
        }
        if let obj = defaults.object(forKey: "maxDataAgeDays"), let value = obj as? Int {
            DataSource.maxDataAgeDays = value
        }
        if let obj = defaults.object(forKey: "imageSize"), let value = obj as? Int {
            DataSource.imageSize = value
        }
        if let obj = defaults.object(forKey: "pageSize"), let value = obj as? Int {
            DataSource.pageSize = value
        }
        
        if let value = defaults.string(forKey: "visualizerUrl") {
            DataSource._visualizerUrl = value
        }
        if let value = defaults.string(forKey: "brandInfoUrl") {
            DataSource._brandInfoUrl = value
        }
        if let value = defaults.string(forKey: "sceneBaseUrl") {
            DataSource._sceneBaseUrl = value
        }
        if let value = defaults.string(forKey: "sceneDataUrl") {
            DataSource._sceneDataUrl = value
        }
        if let value = defaults.string(forKey: "productDetailsUrl") {
            DataSource._productDetailsUrl = value
        }
    }
    
    private func getRemoteConfig() {
        DispatchQueue(label: "server-sync").async {
            do {
                let data = try Data(contentsOf: DataSource.configUrl, options: .mappedIfSafe)
                let jsonResult = try JSONSerialization.jsonObject(with: data, options: .mutableLeaves)
                self.handleConfigData(jsonResult)
            }
            catch {
                print("Remote config not found")
            }
        }
    }
    
    private func handleConfigData(_ jsonResult:Any) {
        guard let config = jsonResult as? Dictionary<String, AnyObject> else {
            print("remote config not parseable, ignoring")
            return
        }
        
        let defaults = UserDefaults.standard
        print("Parsing remote configuration")
        
        if let value = config["forceUpdate"] as? Bool {
            if (value) {
                AppUpdater.shared.showUpdate(withConfirmation: false)
            }
        }
        
        if let value = config["checkForUpdates"] as? Bool {
            if (value) {
                AppUpdater.shared.showUpdate(withConfirmation: true)
            }
        }
        
        if let value = config["cambrianWebURL"] as? String {
            if (DataSource.cambrianWebURL != value) {
                print("cambrianWebURL is now '\(value)'")
                DataSource.cambrianWebURL = value
                defaults.set(DataSource.cambrianWebURL, forKey: "cambrianWebURL")
            }
        }
        
        if let value = config["configUrl"] as? String {
            if (DataSource._configUrl != value) {
                DataSource._configUrl = value
                print("configUrl is now '\(value)'")
                defaults.set(value, forKey: "configUrl")
            }
        }
                
        if let value = config["webSource"] as? String {
            if (DataSource.webSource != value) {
                DataSource.webSource = value
                print("webSource is now '\(value)'")
                defaults.set(value, forKey: "webSource")
            }
        }
        
        if let value = config["baseImagePath"] as? String {
            if (DataSource.baseImagePath != value) {
                DataSource.baseImagePath = value
                print("baseImagePath is now '\(value)'")
                defaults.set(value, forKey: "baseImagePath")
            }
        }
        
        if let value = config["productGroup"] as? String {
            if (DataSource.productGroup != value) {
                DataSource.productGroup = value
                print("productGroup is now '\(value)'")
                defaults.set(value, forKey: "productGroup")
            }
        }
        
        if let value = config["adsEnabled"] as? Bool {
            if (adsEnabled != value) {
                adsEnabled = value
                print("In app ads are now \(value ? "ENABLED" : "DISABLED")")
                defaults.set(value, forKey: "adsEnabled")
            }
        }
        
        if let value = config["maxDataAgeDays"] as? Int {
            if (DataSource.maxDataAgeDays != value) {
                DataSource.maxDataAgeDays = value
                print("maxDataAgeDays is now \(value)")
                defaults.set(value, forKey: "maxDataAgeDays")
            }
        }
        
        if let value = config["imageSize"] as? Int {
            if (DataSource.imageSize != value) {
                DataSource.imageSize = value
                print("imageSize is now \(value)")
                defaults.set(value, forKey: "imageSize")
            }
        }
        
        if let value = config["pageSize"] as? Int {
            if (DataSource.pageSize != value) {
                DataSource.pageSize = value
                print("pageSize is now \(value)")
                defaults.set(value, forKey: "pageSize")
            }
        }
        
        if let value = config["visualizerUrl"] as? String {
            if (DataSource._visualizerUrl != value) {
                DataSource._visualizerUrl = value
                print("visualizerUrl is now '\(value)'")
                defaults.set(value, forKey: "visualizerUrl")
            }
        }
        
        if let value = config["brandInfoUrl"] as? String {
            if (DataSource._brandInfoUrl != value) {
                DataSource._brandInfoUrl = value
                print("brandInfoUrl is now '\(value)'")
                defaults.set(value, forKey: "brandInfoUrl")
            }
        }
        
        if let value = config["sceneBaseUrl"] as? String {
            if (DataSource._sceneBaseUrl != value) {
                DataSource._sceneBaseUrl = value
                print("sceneBaseUrl is now '\(value)'")
                defaults.set(value, forKey: "sceneBaseUrl")
            }
        }
        
        if let value = config["sceneDataUrl"] as? String {
            if (DataSource._sceneDataUrl != value) {
                DataSource._sceneDataUrl = value
                print("sceneDataUrl is now '\(value)'")
                defaults.set(value, forKey: "sceneDataUrl")
            }
        }
        
        if let value = config["productDetailsUrl"] as? String {
            if (DataSource._productDetailsUrl != value) {
                DataSource._productDetailsUrl = value
                print("productDetailsUrl is now '\(value)'")
                defaults.set(value, forKey: "productDetailsUrl")
            }
        }
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
