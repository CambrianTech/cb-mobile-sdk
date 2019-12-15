//
//  DataSource.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/12/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import Alamofire
import RealmSwift

class DataSource {
    
    let jsonPath = "datasource.json"
    let webSource:String
    let productGroup = "shawfloors"
    
    static let baseImagePath = "https://shawfloors.scene7.com/is/image";
    static let imageSize = 320
    static let pageSize = 1000
    
    let realm = try! Realm()
    
    var topLevelCategories: [ProductCategory] = []
    var jsonCategories:Dictionary<String, Dictionary<String, String>> = Dictionary<String, Dictionary<String, String>>()
    
    private init() {
        if let path = Bundle.main.path(forResource: jsonPath, ofType: nil) {
            do {
                let data = try Data(contentsOf: URL(fileURLWithPath: path), options: .mappedIfSafe)
                let jsonResult = try JSONSerialization.jsonObject(with: data, options: .mutableLeaves)
                
                guard let parsed = jsonResult as? Dictionary<String, AnyObject> else {
                    fatalError("cannot parse json data")
                }
                
                guard let source = parsed["source"] as? String, let categories = parsed["categories"] as? Array<AnyObject> else {
                    fatalError("no source attribute in json data")
                }
                
                self.webSource = source
                for categoryJson in categories {
                    let category = categoryJson as! Dictionary<String, String>
                    jsonCategories[category["name"]!] = category
                }
                self.topLevelCategories = parseProductCategories(categories)
                return
            }
            catch {
                fatalError("required datasource caused error")
            }
        }
        fatalError("cannot find required datasource")
    }
    
    private static var _client: DataSource?
    
    static var current: DataSource {
        get {
            if let datasource = DataSource._client {
                return datasource
            }
            let datasource = DataSource()
            DataSource._client = datasource
            return datasource
        }
    }
    
    private func parseProductCategories(_ categoriesJSON:Array<AnyObject>) -> [ProductCategory] {
        var categories: [ProductCategory] = []
        for categoryJson in categoriesJSON {
            let category = ProductCategory(categoryJson as! Dictionary<String, AnyObject>)
            categories.append(category)
            try! realm.write {
                realm.add(category)
            }
        }
        return categories
    }
    
    public class func encodeUrl(_ string:String) -> String {
        return string.addingPercentEncoding(withAllowedCharacters:NSCharacterSet.urlQueryAllowed)!
    }
    
    public class func getThumbnailPath(_ UniqueId:String, imageSize:Int=320) -> String {
        return "\(baseImagePath)/ShawIndustries/\(UniqueId)_MAIN?fit=crop&wid=\(imageSize)&hei=\(imageSize)&fmt=jpg"
    }
}
