//
//  ProductStyle.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

class ProductColor: Object {
    override static func ignoredProperties() -> [String] {
        return ["directoryPath", "thumbnailImage", "jsonString"]
    }
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    //drawable
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var orderIndex = 0
    
    let parents = LinkingObjects(fromType: Product.self, property: "colors")
    let variations = List<ProductVariation>()
    
    var directoryPath:String {
        
        guard let product = parents.first else {
            fatalError("Product Style must have product!")
        }
        
        let basePath = product.directoryPath
        let dir = name.count > 0 ? name.lowercased().trim() : code
        let directoryPath = "\(basePath)/colors/\(dir)"
        
        return directoryPath
    }
    
    var thumbnailPath:URL? {
        let pathString = "\(directoryPath)/Thumbnail.jpg".addingPercentEncoding(withAllowedCharacters: .urlQueryAllowed)!
        return URL(string: pathString)
    }
    
    func loadData(progress: @escaping (Int, Int) -> Void, completion: @escaping (Bool) -> Void) {
        var toLoad = self.variations.count
        
        if (toLoad == 0) {
            completion(true)
            return
        }
        
        var totalBytes: [Int] = []
        var receivedBytes: [Int] = []
        
        for variation in self.variations {
            let index = receivedBytes.count
            totalBytes.append(0)
            receivedBytes.append(0)
            
            variation.loadData(progress: { (receivedSize, expectedSize) in
                
                totalBytes[index] = max(expectedSize, receivedSize)
                receivedBytes[index] = receivedSize
                let totalBytesReceived = receivedBytes.reduce(0, +)
                let totalBytesAvailable = totalBytes.reduce(0, +)

                progress(totalBytesReceived, totalBytesAvailable)
            }) { (completed) in
                toLoad -= 1
                if !completed {
                    completion(false)
                }
                else if (toLoad == 0) {
                    completion(true)
                }
            }
        }
    }
    
    var jsonString:String {
        let variationsJson:String = "[\(self.variations.compactMap({$0.jsonString}).joined(separator:","))]"
        return "{\"name\":\"\(name)\", \"variations\":\(variationsJson)}"
    }
}
