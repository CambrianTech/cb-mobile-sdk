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
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var orderIndex = 0
    @objc dynamic var basePath = ""
    @objc dynamic var thumbnailPath:String = ""
    @objc dynamic var previewPath:String = ""
    @objc dynamic var jsonString:String = ""
    
    static func getDataUrl() -> URL {
        return DataSource.sceneDataUrl
    }
    
    static func parseObjects<Element>(data: Dictionary<String, AnyObject>) -> [Element] where Element : CBDataObject {
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
        
        return scenes as! [Element]
    }
    
    static func sync() {
        super.sync { (objects:[SceneLocation]) in}
    }
    
//    class var all : [SceneLocation] {
//        get {
//            let realmResults = DataSource.current.realm.objects(SceneLocation.self)
//            return Array(realmResults);
//        }
//    }
    
}
