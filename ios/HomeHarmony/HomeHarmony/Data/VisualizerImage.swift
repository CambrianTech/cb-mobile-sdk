//
//  VisualizerImage.swift
//  Prestige
//
//  Created by joseph on 3/9/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import Realm
import RealmSwift


class VisualizerImage : Object {
    
    deinit {
        //print("Destroying image at path \(_directoryPath)")
    }
    
    @objc dynamic var imageID = UUID().uuidString
    override class func primaryKey() -> String? { return "imageID"}
    
    @objc dynamic var created = Date()
    @objc dynamic var modified = Date()
    
    @objc dynamic var name = ""
    
    let assets = List<Asset>()
    
    @objc dynamic var project:VisualizerProject!
    
    convenience public init(project: VisualizerProject) {
        self.init()
        self.project = project
    }
    
    
    //non RLM objects
    override static func ignoredProperties() -> [String] {
        return ["cbImage", "_cbImage", "directoryPath"]
    }
    
    
    func markModified() {
        try! self.realm?.write({
            self.modified = Date()
            self.project?.modified = Date()
        })
    }
    
    @objc dynamic var thumbnail: UIImage? {
        get {
            if let path = self.directoryPath?.path {
                return CBRemodelingScene.getThumbnail(path)
            }
            return nil
        }
    }
    
    @objc dynamic var originalImagePath: URL? {
        get {
            if let path = self.directoryPath {
                return path.appendingPathComponent("scene.png")
            }
            return nil
        }
    }
    
    @objc dynamic var previewImagePath: URL? {
        get {
            if let path = self.directoryPath {
                return path.appendingPathComponent("preview.jpg")
            }
            return nil
        }
    }
    
    @objc dynamic var previewImageExists: Bool {
        get {
            if let path = previewImagePath?.path {
                return FileManager.default.fileExists(atPath: path)
            }
            return false
        }
    }
    
    @objc dynamic var originalImageExists: Bool {
        get {
            if let path = originalImagePath?.path {
                return FileManager.default.fileExists(atPath: path)
            }
            return false
        }
    }
    
    /*
    dynamic var originalImage: UIImage? {
        get {
            if let path = self.directoryPath?.path {
                return CBRemodelingScene.getOriginal(path)
            }
            return nil
        }
    }
    
    dynamic var previewImage: UIImage? {
        get {
            if let path = self.directoryPath?.path {
                return CBRemodelingScene.getPreview(path)
            }
            return nil
        }
    }
 */
    
    @objc dynamic var beforeAfterImage: UIImage? {
        get {
            if let path = self.directoryPath?.path {
                return CBRemodelingScene.getBeforeAfter(path, isHorizontal: true)
            }
            return nil
        }
    }
 
    
    
    @objc dynamic var directoryPath:URL? {
        get {
            return project?.directoryPath.appendingPathComponent(imageID)
        }
    }
    
    
    static func getImage(_ id: String) -> VisualizerImage {
        guard
            let context = DataController.sharedInstance.projectContext
            else { fatalError() }

        var image = context.object(ofType: VisualizerImage.self, forPrimaryKey: id)
        if image == nil {
            //print("id returned no results, creating new image")
            image = VisualizerImage()
            image?.imageID = id
            image?.project = VisualizerProject.currentProject
        }
        return image!
    }
    
    
    func deleteImage(_ completion: (() -> Void)? = nil) {
        try! realm?.write({
            realm?.delete(self)
            
            if let block = completion {
                block()
            }
        })
    }
}
