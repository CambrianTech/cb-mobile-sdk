//
//  CBDataObject.swift
//  FloorAR
//
//  Created by Joel Teply on 1/4/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift
import Alamofire

protocol CBDataObjectProtocol {
    static var shared:CBDataObject {get}
    
    func needsUpdate() -> Bool
    func getDataUrl() -> URL?
    func parseObjects(data:Dictionary<String, AnyObject>, realm:Realm)
    func getAllChildObjects() -> [CBDataObject] 
}

typealias CBDataObject = _CBDataObject & CBDataObjectProtocol

class _CBDataObject: Object, HistoryItem {
    override static func ignoredProperties() -> [String] {
        return ["thumbnailPath"]
    }
    
    override class func primaryKey() -> String? { return "code"}
    
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var updated = Date()
    @objc dynamic var orderIndex = 0
    @objc dynamic var jsonString:String = ""
    
    func getName() -> String {
        return name
    }
    
    func getParent() -> HistoryItem? {
        return nil
    }
    
    var thumbnailPath: String {
        get
        {
            if let path = self.jsonString.jsonData["thumbnailPath"] as? String, path.starts(with: "bundle") {
                return Bundle.main.url(forResource: path, withExtension: nil)!.absoluteString
            }
            return DataSource.getThumbnailPath(self.code)
        }
    }
    
    //public func objects<Element>(_ type: Element.Type) -> RealmSwift.Results<Element> where Element : RealmSwift.Object
    private static var this:CBDataObject.Type {
        get {
            guard let this = self as? CBDataObject.Type else {
                fatalError("Not a CBDataObject, impossible?")
            }
            return this
        }
    }
    
    var isTopLevel:Bool {
        get {
            return type(of: self).this.shared == self
        }
    }
    
    private let s = DispatchSemaphore(value: 1)
    func sync(_ completion: (() -> Void)? = nil) {
        
        _ = s.wait(timeout: DispatchTime.distantFuture)
        defer { s.signal() }
        
        let ele = self as! CBDataObject
        
        if (!ele.needsUpdate()) {
            if let completion = completion {
                completion()
            }
        } else if let url = ele.getDataUrl() {
            print("Synchronizing object \(self.name)")
            DispatchQueue.global(qos: .background).async {
                AF.request(url).responseJSON { response in
                    if let json = response.value as? Dictionary<String, AnyObject> {
                        DispatchQueue.main.async {
                            let realm = DataSource.current.realm
                            try! realm.write {
                                ele.parseObjects(data:json, realm:realm)
                                self.updated = Date()
                            }
                            if let completion = completion {
                                completion()
                            }
                        }
                    }
                }
            }
        }
    }
    
    static func sync(_ completion: (() -> Void)?=nil) {
        let this = self as! CBDataObject.Type
        this.shared.sync(completion)
    }
    
    func syncTree(_ completion: (() -> Void)? = nil) {
        self.sync({
            let ele = self as! CBDataObject
            let children = ele.getAllChildObjects()
            for child in children {
                child.syncTree()
            }
        })
    }
    
    static func syncTree(_ completion: (() -> Void)?=nil) {
        let this = self as! CBDataObject.Type
        print("Synchronizing all objects")
        this.shared.syncTree(completion)
        print("Synchronizing completed. All objects are up to date.")
    }
}
