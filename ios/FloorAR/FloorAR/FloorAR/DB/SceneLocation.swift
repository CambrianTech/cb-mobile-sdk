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
    @objc dynamic var previewPath:String = ""
    
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
        let objects = SceneLocation.all()
        return objects.count == 0
    }
    
    func getDataUrl() -> URL? {
        return DataSource.sceneDataUrl
    }
    
    func parseObjects(data: Dictionary<String, AnyObject>) {
        guard let dict = data["scenes"] as? Array<Dictionary<String,AnyObject>> else {
            fatalError("no source attribute in json data")
        }
        
        var scenes:[CBDataObject] = []
        try! DataSource.current.realm.write {
            for sceneData in dict {
                let scene = SceneLocation(sceneData)
                DataSource.current.realm.add(scene)
                scenes.append(scene)
            }
        }
    }
}
