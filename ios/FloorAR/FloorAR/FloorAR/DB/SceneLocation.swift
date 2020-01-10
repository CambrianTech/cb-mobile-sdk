//
//  SceneLocation.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/24/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit

import Foundation
import RealmSwift

class SceneLocation: CBDataObject {
        
    @objc dynamic var basePath = ""
    @objc dynamic var previewName = ""
    @objc dynamic var thumbnailName = ""
    
    let scenes = List<SceneLocation>()
    
    private static var _shared = SceneLocation()
    static var shared:CBDataObject {
        get {
            return _shared
        }
    }
    
    static func all() -> [SceneLocation] {
        let realmResults = DataSource.current.realm.objects(SceneLocation.self)
        return Array(realmResults);
    }
    
    func needsUpdate() -> Bool {
        if (Date().days(from: self.updated) > max(1, DataSource.maxDataAgeDays / 5)) {
            return true
        }
        return false
    }
    
    func getDataUrl() -> URL? {
        return DataSource.sceneDataUrl
    }
    
    func parseObjects(data: Dictionary<String, AnyObject>, realm:Realm) {
        guard let dict = data["scenes"] as? Array<Dictionary<String,AnyObject>> else {
            fatalError("no source attribute in json data")
        }
        
        self.scenes.removeAll()
        for sceneData in dict {
            let scene = SceneLocation(sceneData)
            realm.add(scene, update: .modified)
            self.scenes.append(scene)
        }
        
    }
    
    func getAllChildObjects() -> [CBDataObject] {
        let scenes: [CBDataObject] = self.scenes.map { $0 as CBDataObject }
        return scenes
    }
}
