//
//  Constants.swift
//  HarmonyApp
//
//  Created by joseph on 4/26/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
//import Realmswift



#if IS_PRESTIGE
    let appColor = UIColor(red: 0.0/255.0, green: 58.0/255.0, blue: 96.0/255.0, alpha: 1.0)
 #else
    let appColor = UIColor(red: 0, green: 99.0/255.0, blue: 153.0/255.0, alpha: 1.0)
#endif

let cambrianBlue = UIColor(red: 38/255.0, green: 153/255.0, blue: 251/255.0, alpha: 1.0)

func hasPaint() -> Bool {
    if let realm = DataController.sharedInstance.brandContext {
        let predicate = NSPredicate(format: "type == %d",
                                    CBAssetType.paint.rawValue)
        let items = realm.objects(BrandItem.self).filter(predicate)
        if (!items.isEmpty) { return true }
    }
    return false
}

func hasFlooring() -> Bool {
    if let realm = DataController.sharedInstance.brandContext {
        let predicate = NSPredicate(format: "type == %d",
                                    CBAssetType.floor.rawValue)
        let items = realm.objects(BrandItem.self).filter(predicate)
        if (!items.isEmpty) { return true }
    }
    return false
}

func getTargetName() -> String {
    return Bundle.main.infoDictionary?["CFBundleName"] as! String
}
