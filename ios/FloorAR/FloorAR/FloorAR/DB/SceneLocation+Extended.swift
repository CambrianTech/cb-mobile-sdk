//
//  SceneLocation+Extended.swift
//  FloorAR
//
//  Created by Joel Teply on 1/2/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import Foundation

extension SceneLocation {
    
    convenience init (_ json:Dictionary<String, AnyObject>) {
        self.init()
        
        self.jsonString = json.jsonString
        self.code = json["name"] as! String
        self.name = json["displayName"] as! String
        self.thumbnailPath = json["thumbnail"] as! String
        self.previewPath = json["preview"] as! String
    }
    
    var thumbnailUrl : URL? {
        return URL(string: thumbnailPath)
    }
    
    class var all : [SceneLocation] {
        get {
            guard let realmResults = DataController.sharedInstance.productContext?.objects(SceneLocation.self) else { return [] }
            return Array(realmResults);
        }
    }
    
    class func sync(_ completion: @escaping () -> Void) {
        if (self.all.count > 0) {
            completion()
        } else {
            
            DispatchQueue.global(qos: .background).async {
                
                do {
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
                
//                Product.loadProducts(categoryID) { (products) in
//                    DispatchQueue.main.async {
//                        try! DataSource.current.realm.write {
//                            self.products.append(objectsIn: products)
//                        }
//                        completion()
//                    }
//                }
            }
        }
    }
}
