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
    func parseObjects(data:Dictionary<String, AnyObject>)
}

typealias CBDataObject = _CBDataObject & CBDataObjectProtocol

class _CBDataObject: Object {
    
    @objc dynamic var id = UUID().uuidString
    override class func primaryKey() -> String? { return "id"}
    
    override static func ignoredProperties() -> [String] {
        return ["directoryPath", "thumbnailImage"]
    }
    
    @objc dynamic var name = ""
    @objc dynamic var code = ""
    @objc dynamic var updated = Date()
    @objc dynamic var orderIndex = 0
    @objc dynamic var thumbnailPath:String = ""
    @objc dynamic var jsonString:String = ""
    
    //public func objects<Element>(_ type: Element.Type) -> RealmSwift.Results<Element> where Element : RealmSwift.Object
    private static var this:CBDataObject.Type {
        get {
            guard let this = self as? CBDataObject.Type else {
                fatalError("Not a CBDataObject, impossible?")
            }
            return this
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
            DispatchQueue.global(qos: .background).async {
                AF.request(url).responseJSON { response in
                    if let json = response.value as? Dictionary<String, AnyObject> {
                        DispatchQueue.main.async {
                            ele.parseObjects(data:json)
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
    
    class func first<Element>() -> Element? where Element : RealmSwift.Object {
        let realmResults = DataSource.current.realm.objects(Element.self)
        return realmResults.first
    }
    
    class func random<Element>() -> Element? where Element : RealmSwift.Object {
        let realmResults = DataSource.current.realm.objects(Element.self)
        let index = Int (arc4random_uniform(UInt32(realmResults.count)));
        return realmResults[index];
    }
}
