//
//  CBDataObject.swift
//  FloorAR
//
//  Created by Joel Teply on 1/4/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import Foundation
import RealmSwift

protocol CBDataObjectProtocol {
    static func getDataUrl() -> URL
    static func parseObjects<Element>(data:Dictionary<String, AnyObject>) -> [Element] where Element : CBDataObject
}

typealias CBDataObject = _CBDataObject & CBDataObjectProtocol

class _CBDataObject: Object {
    
    //public func objects<Element>(_ type: Element.Type) -> RealmSwift.Results<Element> where Element : RealmSwift.Object
    
    static func all<Element>() -> [Element] where Element : RealmSwift.Object {
        let realmResults = DataSource.current.realm.objects(Element.self)
        return Array(realmResults);
    }

    private static var this:CBDataObject.Type {
        get {
            guard let this = self as? CBDataObject.Type else {
                fatalError("Not a CBDataObject, impossible?")
            }
            return this
        }
    }
    
    private static let s = DispatchSemaphore(value: 1)
    static func sync<Element>(_ completion: ((_ objects:[Element]) -> Void)?) where Element : CBDataObject {
        
        _ = s.wait(timeout: DispatchTime.distantFuture)
        defer { s.signal() }
        
        let objects:[Element] = this.all()
        if (objects.count > 0) {
            if let completion = completion {
                completion(objects)
            }
        } else {
            DispatchQueue.global(qos: .background).async {
                do {
                    //print(this.dataUrl)
                    let data = try Data(contentsOf: this.getDataUrl(), options: .mappedIfSafe)
                    let jsonResult = try JSONSerialization.jsonObject(with: data, options: .mutableLeaves)
                    
                    guard let parsed = jsonResult as? Dictionary<String, AnyObject> else {
                        fatalError("cannot parse json data")
                    }

                    DispatchQueue.main.async {
                        let objects:[Element] = this.parseObjects(data:parsed)
                        if let completion = completion {
                            completion(objects)
                        }
                    }
                }
                catch {
                    fatalError("required datasource caused error")
                }
            }
        }
    }
}
