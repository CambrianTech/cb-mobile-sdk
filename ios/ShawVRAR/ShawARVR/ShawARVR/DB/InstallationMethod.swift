//
//  ProductStyle.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit

import Foundation
import RealmSwift

class InstallationMethod: Object {
    override static func ignoredProperties() -> [String] {
        return ["directoryPath", "thumbnailImage"]
    }
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    //drawable
    @objc dynamic var name = ""
    @objc dynamic var orderIndex = 0
    @objc dynamic var thumbnail = ""
    
    var thumbnailImage:UIImage? {
        let name = NSURL(fileURLWithPath: thumbnail).deletingPathExtension?.lastPathComponent ?? ""
        return UIImage(named: name)
    }
    
    var jsonString:String {
        return "{\"name\":\"\(name)\"}"
    }
}

