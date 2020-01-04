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
                AF.request(this.getDataUrl()).responseJSON { response in
                    if let json = response.value as? Dictionary<String, AnyObject> {
                        DispatchQueue.main.async {
                            let objects:[Element] = this.parseObjects(data:json)
                            if let completion = completion {
                                completion(objects)
                            }
                        }
                    }
                }
            }
        }
    }
}
