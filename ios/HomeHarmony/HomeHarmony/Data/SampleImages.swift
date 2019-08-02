//
//  SamplesData.swift
//  Prestige
//
//  Created by Joel Teply on 7/18/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import Alamofire
import SwiftyJSON
import Kingfisher
import Zip
import RealmSwift
import Realm

internal protocol SampleImageDelegate : NSObjectProtocol {
    func imagesLoaded()
    func zipDownloaded()
}

class SampleImages {
    
    weak internal var delegate:SampleImageDelegate?
    var version:String?
    
    var rooms: [SampleRoom] = []
    let baseURL = "https://s3.us-east-2.amazonaws.com/sample-images-v3/"
    
    func ensureDirectoryExists(_ path:String) -> Bool {
        if !FileManager.default.fileExists(atPath: path) {
            do {
                try FileManager.default.createDirectory(atPath: path, withIntermediateDirectories: true, attributes: nil)
            } catch {
                //print(error)
                return false
            }
        }
        return true
    }

    
    func loadJson() {
        let jsonURL = "\(baseURL)list.json"
        //print("json url: \(jsonURL)")
        Alamofire.request(jsonURL).responseData { response in
            debugPrint("All Response Info: \(response)")
            
            if let data = response.result.value {
                self.parseJson(data)
            }
        }
    }
    
    fileprivate func parseJson(_ data:Data) {
        do {
            let json:JSON = try JSON(data: data)
            if let rooms = json["rooms"].array {
                for room in rooms {
                    let name = room["name"].stringValue
                    //print(name)
                    self.rooms.append(SampleRoom(name: room["name"].stringValue))
                    if let numImages = room["numImages"].int {
                        for i in 0..<numImages {
                            let urlString = ("\(baseURL)\(name)/\(i)/scene.png").addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed)
                            //print(urlString)
                            if let url = URL(string: urlString!) {
                                //print(url)
                                self.rooms.last?.addImage(url: url)
                            }
                        }
                    }
                }
            }
        } catch { }
        
        delegate?.imagesLoaded()
    }
    
    func getSampleDirectory() -> URL {
        return try! FileManager().url(for: .documentDirectory, in: .userDomainMask, appropriateFor: nil, create: false)
    }
    
    func downloadZip(room: String, index: String) {

        let urlString = "\(baseURL)\(room)/\(index).zip"
        let url = URL(string: urlString.addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed)!)
        let projectDir = VisualizerProject.currentProject.directoryPath
        let id = UUID().uuidString
        let imageDir = projectDir.appendingPathComponent(id)
        let localZipPath = projectDir.appendingPathComponent("sample.zip")
        //print("URL: \(url)")
        //print("Local Path: \(localZipPath)")
        initiateFileDownload(url!, savePath: localZipPath, overwriteFiles: true, completionCallback: { (success) in
            if(success) {
                
                try! Zip.unzipFile(localZipPath, destination: imageDir, overwrite: true, password: "", progress: { (progress) -> () in
                    //print("unzipping... \(progress)")
                })
                
                VisualizerProject.currentProject.appendImage(id)
                
                do {
                    try FileManager.default.removeItem(at: localZipPath)        //delete zip to clean up
                } catch {
                    //print(error)
                }
                self.delegate?.zipDownloaded()
            }
        })
    }
    
    
    func initiateFileDownload(_ remotePath:URL, savePath:URL,
                              overwriteFiles:Bool,
                              completionCallback:@escaping ((_ success: Bool) -> Void),
                              progressCallback:((_ progress: Float) -> Void)? = nil) {
        
        if !overwriteFiles && FileManager.default.fileExists(atPath: savePath.path) {
            completionCallback(true)
            return
        }
        
        let parentDir = savePath.deletingLastPathComponent().path
        if ensureDirectoryExists(parentDir) {
            //print("directory exists")
        }
        
        let destination: DownloadRequest.DownloadFileDestination = { _, _ in
            return (savePath, [.removePreviousFile])
        }
        
        Alamofire.download(remotePath, to: destination)
            .downloadProgress { progress in
                //print("Download Progress: \(progress.fractionCompleted)")
                displayProgress("Downloading...", progress: progress.fractionCompleted)
            }
            .responseData { response in
                //print("initiateFileDownload: destination: \(savePath.path)")
                
                let result = FileManager.default.fileExists(atPath: savePath.path)
                //print("initiateFileDownload: file exists \(result)")
                
                DispatchQueue.main.async {
                    completionCallback(result)
                }
        }
    }
}


class SampleRoom {
    var name:String
    var images = [URL]()
    
    init(name: String) {
        self.name = name
    }
 
    func addImage(url: URL) {
        images.append(url)
    }
}
