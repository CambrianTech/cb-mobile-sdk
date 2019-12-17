//
//  Extensions.swift
//  ShawARVR
//
//  Created by Joel Teply on 12/15/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import Foundation

extension String {
    var jsonData : [String: AnyObject]
    {
        get {
            if let data = self.data(using: String.Encoding.utf8) {
                do {
                    let json = try JSONSerialization.jsonObject(with: data, options: JSONSerialization.ReadingOptions.allowFragments) as? [String: AnyObject]
                    return json!

                } catch {
                    print("Error converting to JSON")
                }
            }
            return Dictionary<String, AnyObject>()
        }
    }
}

extension Dictionary {
    var jsonString : String
    {
        get{
            do {
                let jsonData: Data = try JSONSerialization.data(withJSONObject: self, options: .prettyPrinted)
                return String.init(data: jsonData, encoding: .utf8)!
            }
            catch
            {
                print("Error converting JSON")
                return "{}"
            }
        }
    }
}
