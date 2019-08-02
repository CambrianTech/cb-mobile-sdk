//
//  ProductVariation.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/24/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import Foundation
import RealmSwift
import SDWebImage

class ProductVariation: Object {
    override static func ignoredProperties() -> [String] {
        return ["directoryPath", "thumbnailImage", "jsonString"]
    }
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    //drawable
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var orderIndex = 0
    
    let parents = LinkingObjects(fromType: ProductColor.self, property: "variations")
    
    var directoryPath:String {
        
        guard let productColor = parents.first else {
            fatalError("Product Style must have product!")
        }
        
        let basePath = productColor.directoryPath
        let directoryPath = "\(basePath)/variations/\(name.lowercased().trim())"
        
        return directoryPath
    }
    
    private var remoteDiffusePath:URL {
        return DataController.sharedInstance.getRemoteImagePath(path: directoryPath, named: "Base_Color.jpg")!
    }
    
    private var remoteNormalPath:URL {
        return DataController.sharedInstance.getRemoteImagePath(path: directoryPath, named: "Normal.jpg")!
    }
    
    private var remoteRoughnessPath:URL {
        return DataController.sharedInstance.getRemoteImagePath(path: directoryPath, named: "Roughness.jpg")!
    }
    
    var diffusePath:String {
        return DataController.sharedInstance.getLocalImagePath(remoteDiffusePath)!
    }

    var normalPath:String {
        return DataController.sharedInstance.getLocalImagePath(remoteNormalPath)!
    }

    var roughnessPath:String {
        return DataController.sharedInstance.getLocalImagePath(remoteRoughnessPath)!
    }
    
    func loadData(progress: @escaping (Int, Int) -> Void, completion: @escaping (Bool) -> Void) {
        DataController.sharedInstance.getCachedImages(remotePaths: [remoteDiffusePath, remoteNormalPath, remoteRoughnessPath],
                                                      progress: progress,
                                                      completion: completion)
    }
    
    var jsonString:String {
        return "{\"name\":\"\(name)\", \"diffusePath\":\"\(diffusePath)\", \"normalPath\":\"\(normalPath)\", \"roughnessPath\":\"\(roughnessPath)\"}"
    }
}
