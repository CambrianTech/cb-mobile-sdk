//
//  AWSAssetUtility.swift
//  HarmonyApp
//
//  Created by Joel Teply on 10/20/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import AWSS3
import Kingfisher
import Alamofire

class AWSAssetUtility : AWSUtility {
    
    var dbAssetsDir: URL?   // contain Document/dbassets
    var localPath: URL? // contain Document/dbassets/items
    var assetDownloadPath: URL? // path to the actual item being downloaded (item/itemname/Base_color.jpg)
    var assetPath: String?
    var item: BrandItem?
    
    convenience init(_ item: BrandItem) {
        
        self.init()
        
        self.item = item
        self.assetPath = item.assetPath
        
        let documentsURL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
        self.dbAssetsDir = documentsURL.appendingPathComponent("dbassets")
        self.localPath = dbAssetsDir?.appendingPathComponent("items")
        
        self.assetDownloadPath = dbAssetsDir?.appendingPathComponent(self.assetPath!)
        do {
            if let dbAssetsDir = self.dbAssetsDir, let localPath = self.localPath ,let assetDownloadPath = self.assetDownloadPath {
                if !(FileManager.default.fileExists(atPath: dbAssetsDir.path)) {
                    try FileManager.default.createDirectory(atPath: dbAssetsDir.path, withIntermediateDirectories: true, attributes: nil)
                    try FileManager.default.createDirectory(atPath: localPath.path, withIntermediateDirectories: true, attributes: nil)
                }
                if !(FileManager.default.fileExists(atPath: assetDownloadPath.path)) {
                    try FileManager.default.createDirectory(atPath: assetDownloadPath.path, withIntermediateDirectories: true, attributes: nil)
                }
            }
        } catch {
            //print("Error: \(error.localizedDescription)")
        }
    }
    
    func downloadPath() -> URL {
        let assetURL = baseURL.appendingPathComponent(self.assetPath!)
        return assetURL
    }
    
    func initiateDownload(completed: @escaping (Bool) -> Void) {
        var downloaded = 0
        var failed = 0
        
        displayProgress("Downloading Asset", progress: 0.05)
        
        let callback = {
            if failed > 0 {
                completed(false)
            } else if downloaded == self.downloadList.count {
                completed(true)
            }
        }
        
        //var totalUnits:Int64 = 0
        var totalTracked = 0
        let totalProgress = Progress()
        
        self.downloadList.forEach { (key) in
            guard let savePath = self.dbAssetsDir?.appendingPathComponent(key) else {
                return
            }
            
            // Download all the files with the assetPath in the file name
            // Get the last component and create a path and download to said path
            
            //print("SAVE PATH: \(savePath.path)")
            
            do {
                try FileManager.default.createDirectory(atPath: savePath.deletingLastPathComponent().path, withIntermediateDirectories: true, attributes: nil)
            }
            catch {
                //print("Error: \(error.localizedDescription)")
                completed(false)
            }
            
            var iAmTracked = false
            var lastUnits:Int64 = 0
            downloadObject(key, savePath: savePath, progressCallback: {(progress) in
                
                if (!iAmTracked) {
                    iAmTracked = true
                    totalTracked += 1
                    totalProgress.totalUnitCount += progress.totalUnitCount
                }
                
                let additionalChange = progress.completedUnitCount - lastUnits
                totalProgress.completedUnitCount += additionalChange
                lastUnits = progress.completedUnitCount
                
                if (totalTracked == self.downloadList.count && totalProgress.fractionCompleted > Double(progressHUD.progress)) {
                    displayProgress("Downloading Asset", progress: totalProgress.fractionCompleted)
                } else {
                    //for first quarter, use total tracked objects
                    displayProgress("Downloading Asset", progress: 0.25 * Double(totalTracked)/Double(self.downloadList.count))
                }
            }) { (success) in
                if (success) {
                    downloaded += 1
                } else {
                    failed += 1
                }
                callback()
            }
        }
    }
    
    func downloadAsset(completed: @escaping (Bool) -> Void) {
        
        displayProgress("Downloading Asset", progress: 0.05)
        
        // Amazon AWS stuff
        AWSS3.register(with: self.configuration, forKey: "items/")
        let s3 = AWSS3.s3(forKey: "items/")
        
        let listRequest: AWSS3ListObjectsRequest = AWSS3ListObjectsRequest()
        listRequest.bucket = s3BucketName
        s3.listObjects(listRequest).continueWith { (task) -> Any? in
            if let error = task.error {
                //print("AWS error: \(error.localizedDescription)")
                completed(false)
                return nil
            }
            guard let _ = task.result?.contents?.last else {
                completed(true) //nothing to do
                return nil
            }
            
            for object in task.result!.contents! {
                // We've already grabbed thumbnails
                if !(object.key?.hasSuffix("Thumbnail.jpg"))! {
                    if let key = object.key, let path = self.assetPath {
                        // If the S3 listed object contains the the asset information, then download
                        if key.range(of: path) != nil {
                            // Build up list of stuff to donwload
                            self.downloadList.append(key)
                        }
                    }
                }
                
            }
            //print(self.downloadList.count)
            self.initiateDownload(completed: { success in
                DispatchQueue.main.async {
                    completed(success)
                }
            })
            
            return nil
        }
    }
}
