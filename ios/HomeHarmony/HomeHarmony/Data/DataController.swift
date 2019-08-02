//
//  DataManager.swift
//  Prestige
//
//  Created by Joel Teply on 6/7/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift

class DataController: NSObject {
    var brandContext: Realm?
    var brandCurrentDataVersion:Double = 0.2
    let schemaVersion:UInt64 = 1
    
    var projectContext: Realm?
    
    
    var workingDirectory = DataController.getWriteDirectory()
    
    override init() {
        super.init()
    
        refreshSyncedDatabases()
        
        self.projectContext = intitializeDataStore("ProjectModel")
    }
    
    func refreshSyncedDatabases() {
        
        self.brandContext = intitializeVersionedDataStore(datastoreName:"BrandModel", bundleVersion:self.brandCurrentDataVersion) { realm in
            self.brandContext = realm
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
        
        //go ahead and load the db
        let downloadUtil = AWSUtility()
        let key = "\(datastoreName).realm"

        downloadUtil.getObjectTags(key) { (success, tags) in
            if let versionString = tags.filter({$0.key == "version" }).first?.value,
                let webVersion = Double(versionString) {

                if (webVersion > existingDataVersion) {
                    //print("\(datastoreName) database needs upgrade from web (\(existingDataVersion) < \(webVersion)), attempting web download.")

                    displayProgress("Downloading database", progress: 0)

                    downloadUtil.downloadObject(key, savePath: storeURL, progressCallback:{ (progress) in
                        displayProgress("Downloading database", progress: progress.fractionCompleted)
                    }) { (success) in

                        if (success) {
                            //print("Download of \(datastoreName) database successful. Wrote new version (\(webVersion)), loading it.")

                            UserDefaults.standard.set(webVersion, forKey: datastoreName)
                            self.brandCurrentDataVersion = webVersion
                        } else {
                            //print("Download of \(datastoreName) database was unsuccessful, loading existing version \(existingDataVersion).")
                        }

                        displayProgress("Downloading new database", progress: 100)

                        DispatchQueue.main.async {
                            hideProgress()
                            completed(self.intitializeDataStore(datastoreName))
                        }
                    }
                } else {
                    //print("Version (\(webVersion)) of \(datastoreName) database is current with web, loading it.")
                    DispatchQueue.main.async {
                        completed(self.intitializeDataStore(datastoreName))
                    }
                }
            } else { //tag request failed
                //maybe no internet, ignore
                DispatchQueue.main.async {
                    completed(self.intitializeDataStore(datastoreName))
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
            if (oldSchemaVersion == 0 && self.schemaVersion == 1) {
                migrateZeroToOne(migration)
            }
        }
        
        do {
            let realm = try Realm(configuration: config)
            return realm
        } catch let error as NSError  {
            print("Realm Error %s", error.localizedDescription);
            fatalError()
        }
        
        return nil
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
