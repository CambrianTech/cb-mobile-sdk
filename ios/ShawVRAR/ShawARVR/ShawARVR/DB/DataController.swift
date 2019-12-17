//
//  DataManager.swift
//  Prestige
//
//  Created by Joel Teply on 6/7/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift
import SDWebImage
import AWSS3

class DataController: NSObject {
    var productContext: Realm?
    var productCurrentDataVersion:Double = 3.0
    let schemaVersion:UInt64 = 3
    
    let bucket = "cambrian-app-logging"
    let access = "REDACTED_AWS_ACCESS_KEY"
    let secret = "REDACTED_AWS_SECRET_KEY"
    
    var workingDirectory = DataController.getWriteDirectory()
    
    var emptyDict: [String: String] = [:]
    
    override init() {
        super.init()
    
        //refreshSyncedDatabases()
        
        _ = SceneLocation() // trigger creation of DB field
        
        #if AUTO_GEN_CATEGORIES
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) { // in half a second...
                DataController.sharedInstance.autoGenFields()
            }
        #endif
    }
    
    func getRemoteImagePath(path:String, named:String) -> URL? {
        if let stringPath = (path + "/" + named).addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed) {
            return URL(string: stringPath)
        }
        return nil
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
    
#if AUTO_GEN_CATEGORIES
    
    func getBundlePath(_ path:String) -> String? {
        if let index = path.index(of: "/categories") {
            let suffix = path.suffix(from: index);
            return "\(Bundle.main.bundleURL.path)\(suffix)"
        }
        return nil
    }
    
    func getWorkingPath(_ path:String) -> String? {
        if let index = path.index(of: "/categories") {
            let suffix = path.suffix(from: index);
            return "\(DataController.sharedInstance.workingDirectory.path)\(suffix)"
        }
        return nil
    }
    
    func copyBundleFiles(_ path:String) {
        
        FileManager.ensureDirectoryExists(atPath: path)
        print("Generated path \(path)")
        
        if let bundlePath = getBundlePath(path) {
            
            if FileManager.default.fileExists(atPath: bundlePath) {
                
                do {
                    var numCreated = 0
                    let srcFileURLs = try FileManager.default.contentsOfDirectory(at: URL(fileURLWithPath: bundlePath, isDirectory: true), includingPropertiesForKeys: nil)
                    // copy files
                    try srcFileURLs.forEach { (url) in
                        if let dest = getWorkingPath(url.path), !FileManager.default.fileExists(atPath: dest) {
                            try FileManager.default.copyItem(at: url, to:  URL(fileURLWithPath:dest))
                            numCreated += 1
                        }
                    }
                    if (numCreated > 0) {
                        print("Copied \(numCreated) files.")
                    }
                } catch {
                    print("Error while enumerating files \(bundlePath): \(error.localizedDescription)")
                }
            }
        }
    }
    
    func autoGenFields() {
        if let categories = productContext?.objects(ProductCategory.self) {
            for category in categories {
                copyBundleFiles(category.directoryPath)
                for product in category.products {
                    copyBundleFiles(product.directoryPath)
                    for color in product.colors {
                        copyBundleFiles(color.directoryPath)
                        for variation in color.variations {
                            copyBundleFiles(variation.directoryPath)
                        }
                    }
                }
            }
        }
        
        print("All directories are up to date")
    }
    
#endif
    
    var assetPath:String {
//        #if AUTO_GEN_CATEGORIES
//            return DataController.sharedInstance.workingDirectory.path
//        #else
//            return Bundle.main.bundleURL.path
//        #endif
        return "https://s3.amazonaws.com/cambrian-data"
        return DataController.sharedInstance.workingDirectory.path
    }
    
    func refreshSyncedDatabases() {
        
//        self.productContext = intitializeVersionedDataStore(datastoreName:"ProductModel", bundleVersion:self.productCurrentDataVersion) { realm in
//            self.productContext = realm
//        }
    }
    
    func uploadDiagnosticImage(_ path:String) {
        if (!FileManager.default.fileExists(atPath: path)) {
            return
        }
        
        let url = NSURL(fileURLWithPath: path)
        let credentials = AWSStaticCredentialsProvider(accessKey: access, secretKey: secret)
        let configuration = AWSServiceConfiguration(region: AWSRegionType.USEast1, credentialsProvider: credentials)
        
        let device_id = (UIDevice.current.identifierForVendor != nil) ? UIDevice.current.identifierForVendor!.uuidString : "unknown"
        
        AWSServiceManager.default().defaultServiceConfiguration = configuration
        
        let data: Data = FileManager.default.contents(atPath: path)!
        let file_name = NSURL(fileURLWithPath: path).lastPathComponent!
        let formatExtension = url.pathExtension!
        let remoteName = "shaw-ios/" + device_id + "/" + file_name
        
        let expression = AWSS3TransferUtilityUploadExpression()
        expression.setValue("public-read", forRequestParameter: "x-amz-acl")
        
        expression.progressBlock = { (task, progress) in
            DispatchQueue.main.async(execute: {
                // Update a progress bar
            })
        }
        
        var completionHandler: AWSS3TransferUtilityUploadCompletionHandlerBlock?
        completionHandler = { (task, error) -> Void in
            DispatchQueue.main.async(execute: {
                // Do something e.g. Alert a user for transfer completion.
                // On failed uploads, `error` contains the error object.
            })
        }
        
        let transferUtility = AWSS3TransferUtility.default()
        transferUtility.uploadData(data, bucket: bucket, key: remoteName, contentType: "image/"+formatExtension, expression: expression, completionHandler: completionHandler).continueWith { (task) -> Any? in
            if let error = task.error {
                print("Error : \(error.localizedDescription)")
            }
            
            //Delete local copy
            do {
                try FileManager.default.removeItem(atPath: path)
            } catch { }
            
            if task.result != nil {
                let url = AWSS3.default().configuration.endpoint.url
                let _ = url?.appendingPathComponent(self.bucket).appendingPathComponent(remoteName)
//                if let absoluteString = publicURL?.absoluteString {
//                    // Set image with URL
//                    print("Uploaded image to ", absoluteString)
//                }
            }
            
            return nil
        }
        
    }
    
    static func getWriteDirectory() -> URL {
        return try! FileManager().url(for: .documentDirectory, in: .userDomainMask, appropriateFor: nil, create: true)
    }
    
    static let sharedInstance = DataController()
    
    func intitializeVersionedDataStore(datastoreName:String, bundleVersion:Double, completed: @escaping (Realm?) -> Void) -> Realm? {
        // This resource is the same name as your xcdatamodeld contained in your project.
        
        /* The directory the application uses to store the Core Data store file.
         This code uses a file named "DataModel.sqlite" in the application's documents directory.
         */
        let storeURL = workingDirectory.appendingPathComponent(datastoreName).appendingPathExtension("realm")
        //print("Store URL: \(storeURL)")
        let exists = FileManager.default.fileExists(atPath: storeURL.path)
        
        let bundleURL = Bundle.main.bundleURL.appendingPathComponent(datastoreName).appendingPathExtension("realm")
        
        //print(FileManager.default.fileExists(atPath: bundleURL.path))
        
        var existingDataVersion = UserDefaults.standard.double(forKey: datastoreName)
        //print("EXISTING DATA VERSION \(existingDataVersion)")
        if (!exists || existingDataVersion < bundleVersion) {
            //bring over from bundle
            
            if (FileManager.default.fileExists(atPath: bundleURL.path)) {
                do {
                    if (exists) {
                        //print("\(datastoreName) database exists locally, but is outdated from bundle. Upgrading (\(existingDataVersion) < \(bundleVersion))")
                        try FileManager.default.removeItem(at: storeURL)
                    } else {
                        //print("\(datastoreName) database does not exist, using bundle version \(bundleVersion)")
                    }
                    try FileManager.default.copyItem(at: bundleURL, to: storeURL)
                    UserDefaults.standard.set(bundleVersion, forKey: datastoreName)
                    existingDataVersion = bundleVersion
                } catch {
                    fatalError("Error loading store: \(error)")
                }
            }
        }
        
        return intitializeDataStore(datastoreName);
    }
    
    func intitializeDataStore(_ datastoreName:String) -> Realm? {
        let storeURL = workingDirectory.appendingPathComponent(datastoreName).appendingPathExtension("realm")
        
        var config = Realm.Configuration.defaultConfiguration
        config.fileURL = storeURL
        config.readOnly = false
        config.schemaVersion = schemaVersion
        config.migrationBlock = { migration, oldSchemaVersion in
            print("Got old schema of \(oldSchemaVersion)")
        }
        
        do {
            let realm = try Realm(configuration: config)
            return realm
        } catch let error as NSError  {
            print("Realm Error \(error.localizedDescription)");
            fatalError()
        }
    }
    
    class func performTransaction(_ object: Object, context:Realm, block:(()->(Bool))? = nil) -> Bool {

        var success = true
        
        let startedTransaction = !context.isInWriteTransaction
        
        if (startedTransaction) {
            context.beginWrite()
        }
        
        if let block = block {
            success = block()
        }
        
        if (startedTransaction) {
            if (success) {
                do {
                    try context.commitWrite()
                } catch {
                    success = false
                }
            } else {
                context.cancelWrite()
            }
        }
        
        return success
    }
}
