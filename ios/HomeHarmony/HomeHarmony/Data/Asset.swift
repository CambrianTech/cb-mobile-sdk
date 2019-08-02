//
//  Asset.swift
//  HarmonyApp
//
//  Created by joseph on 4/18/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift

class Asset: Object {
    
    deinit {
        //print("Destroying image at path \(_directoryPath)")
    }
    
    @objc dynamic var assetID = UUID().uuidString
    override class func primaryKey() -> String? { return "assetID"}
    
    @objc dynamic var created = Date()
    @objc dynamic var modified = Date()
    
    @objc dynamic var itemID: String?
    
    convenience public init(_ id: String) {
        self.init()
        self.assetID = id
    }
    
    func getItem() -> BrandItem? {
        if let id = itemID {
            if let item = BrandItem.itemForID(id) {
                return item
            }
        }
        return nil
    }
    
    var item: BrandItem? {
        get {
            if let id = itemID {
                if let item = BrandItem.itemForID(id) {
                    return item
                }
            }
            return nil
        }
    }
}
