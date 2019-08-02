//
//  ProjectReaderController.swift
//  HarmonyApp
//
//  Created by Jaremy Longley on 8/26/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//
import Foundation
import Branch
import Zip
import SwiftyJSON
import Alamofire
import RealmSwift

class ProjectReaderController : NSObject {
    
    var storageRef: StorageReference!
    var ID : String!
    var project : VisualizerProject!
    var localZipPath : URL!
    var localDir : URL!
    var projectDir: URL!
    var imagePath : URL!
    var jsonUrl : URL!
    var sceneJSON : JSON!
    var isImage: Bool!
    weak var delegate: ProjectReaderDelegate?
    
    init(ID: String, isImage: Bool) {
        self.ID = ID
        print(ID)
        self.isImage = isImage
        self.localDir = DataController.getWriteDirectory().appendingPathComponent("projects")
        if isImage {
            //print("image here")
        } else {
            self.localZipPath = self.localDir.appendingPathComponent(ID).appendingPathExtension("zip")
            self.projectDir = localDir.appendingPathComponent(ID)
        }
    }
    
    func createStorageReference() {
        //Create a reference to the Firebase storage path
        let storage = Storage.storage()
        var downloadRef = StorageReference()
        if self.isImage {
            downloadRef = storage.reference(withPath: "images")
        } else {
            downloadRef = storage.reference(withPath: "projects")
        }
        self.storageRef = downloadRef.child(self.ID)
    }
    
    func downloadZip(timeoutSeconds:Double=30.0, completed: @escaping () -> Void) {
        displayIndeterminateProgress("Loading project")
        if isImage {
            self.createProject()
            //print(self.project.directoryPath.path)
            self.localZipPath = self.localDir.appendingPathComponent(self.project.projectID).appendingPathComponent(self.ID).appendingPathExtension("zip")
            self.projectDir = localDir.appendingPathComponent(self.project.projectID)
            self.localDir = self.projectDir
            //print(self.localDir)
        }
        storageRef.downloadURL { (url, error) in
            if let error = error {
                //print("Error : \(error)")
            } else {
                if let localZipPath = self.localZipPath {
                    initiateFileDownload(url!, savePath: localZipPath, overwriteFiles: true) { (success) in
                        
                        if (success) {
                            try! Zip.unzipFile(localZipPath, destination: self.localDir, overwrite: true, password: "", progress: { (progress) -> () in
                                //print("unzipping... \(progress)")
                            })
                            if !self.isImage {
                                self.createProject()
                            }
                            
                            do {
                                try FileManager.default.removeItem(at: localZipPath)        //delete zip to clean up
                            } catch {
                                //print(error)
                            }
                            self.loopFileStructure()
                            hideProgress()

                            completed()
                        }
                    }
                }
                
            }
        }
    }
    
    func createProject() {
        // Create a project and move the directory
        self.project = VisualizerProject.createProject(name: "Shared Project")
        if (!self.isImage) {
            self.moveProjectDirectory()
        }
        VisualizerProject.currentProject = self.project
    }
    
    func moveProjectDirectory() {
        // Try to move the file to the shared project directory
        do {
            try FileManager.default.moveItem(at: self.projectDir, to: self.project.directoryPath)
            self.projectDir = self.project.directoryPath
        } catch {
            //print("Error Moving files to project directory: \(error)")
        }
    }
    
    func loopFileStructure() {
            // Loop through and get files in directory
            do {
                let directoryContents = try FileManager.default.contentsOfDirectory(at: self.projectDir, includingPropertiesForKeys: nil, options: [])
                
                
                directoryContents.forEach({ (file) in
                    // Loop through folder contents to instantiate images
                    //print("FILE : \(file.path)")
                    let imageDir = file.lastPathComponent
                    let assets = self.readJson(file)
                    let image = VisualizerImage.getImage(imageDir)
                    //print(image.directoryPath?.path)
                    self.project.appendImage(image: image)
                    for asset in assets {
                        if let realm = image.realm {
                            try! realm.write {
                                image.assets.append(asset)
                            }
                        }
                    }
                    self.imagePath = file
                    self.delegate?.updateProjectPath(self.imagePath.path)
                    
                })
            } catch {
                //print("ERROR Reading file contents: \(error)")
            }
        
    }
    
    private func readJson(_ resource: URL) -> List<Asset> {
        var assetList = List<Asset>()
        let file = resource.appendingPathComponent("scene_data.json")
        //print(file.absoluteString)
        do {
            if FileManager.default.fileExists(atPath: file.path)  {
                let data = try? Data(contentsOf: file)
                let json = try JSON(data: data!)
                let assets = json["assets"].arrayValue
                //print(assets)
                if let assets = try json["assets"].arrayValue as? [JSON] {
                    //print(assets.description)
                    assetList = self.getAssets(assets)
                } else {
                    //print("Error reading the assets array")
                }
            } else {
                //print("no file")
            }
        } catch {
            //print(error.localizedDescription)
        }
        return assetList
    }
    
    private func getAssets(_ assets: [JSON]) -> List<Asset> {
        let assetList = List<Asset>()
        //print(assets.count)
        for asset in assets {
            //print(asset)
            // Set random ID so it doesn't find another asset with the exact same ID
            let id = UUID().uuidString
            let userData = asset["userData"].dictionary
            let itemID = userData?.first?.value.stringValue
            let newAsset = Asset(id)
            newAsset.itemID = itemID
            //print(newAsset.getItem()?.name)
            assetList.append(newAsset)
        }
        return assetList
    }
}
