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

class SceneLocation: Object {
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var orderIndex = 0
    @objc dynamic var basePath = ""
    @objc dynamic var thumbnailPath:String = ""
    @objc dynamic var previewPath:String = ""
    @objc dynamic var jsonString:String = ""
    
    class var all : [SceneLocation] {
        get {
            let realmResults = DataSource.current.realm.objects(SceneLocation.self)
            return Array(realmResults);
        }
    }
    
    static let s = DispatchSemaphore(value: 1)
    
    class func sync(_ completion: @escaping () -> Void) {
        _ = s.wait(timeout: DispatchTime.distantFuture)
        defer { s.signal() }
        
        if (self.all.count > 0) {
            completion()
        } else {
            
            DispatchQueue.global(qos: .background).async {
                
                do {
                    print(DataSource.sceneDataUrl)
                    let data = try Data(contentsOf: DataSource.sceneDataUrl, options: .mappedIfSafe)
                    let jsonResult = try JSONSerialization.jsonObject(with: data, options: .mutableLeaves)
                    
                    guard let parsed = jsonResult as? Dictionary<String, AnyObject> else {
                        fatalError("cannot parse json data")
                    }
                    
                    guard let scenes = parsed["scenes"] as? Array<Dictionary<String,AnyObject>> else {
                        fatalError("no source attribute in json data")
                    }

                    DispatchQueue.main.async {
                        try! DataSource.current.realm.write {
                            for sceneData in scenes {
                                let scene = SceneLocation(sceneData)
                                DataSource.current.realm.add(scene)
                            }
                        }
                        completion()
                    }
                    
                    return
                }
                catch {
                    fatalError("required datasource caused error")
                }
            }
        }
    }
}
