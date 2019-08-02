//
//  DataMigration.swift
//  HomeHarmony
//
//  Created by Joel Teply on 10/2/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift

extension Migration {
    func hadProperty(onType typeName: String, property propertyName: String) -> Bool {
        var hasPropery = false
        self.enumerateObjects(ofType: typeName) { (oldObject, _) in
            hasPropery = oldObject?.objectSchema.properties.contains(where: { $0.name == propertyName }) ?? false
            return
        }
        return hasPropery
    }
    
    func renamePropertyIfExists(onType typeName: String, from oldName: String, to newName: String) {
        if (hadProperty(onType: typeName, property: oldName)) {
            renameProperty(onType: typeName, from: oldName, to: newName)
        }
    }
}


//required:
//- Primary Key for class 'BrandCategory' has changed from 'categoryID' to 'id'.
//- Primary Key for class 'BrandItem' has changed from 'itemID' to 'id'.
//- Property 'BrandCategory.categoryID' has been removed.

//not needed:
//- Property 'BrandCategory.blue' has been added.
//- Property 'BrandCategory.green' has been added.
//- Property 'BrandCategory.id' has been added.
//- Property 'BrandCategory.displayItem' has been added.
//- Property 'BrandCategory.red' has been added.
//- Property 'BrandItem.type' has been added.
//- Property 'BrandItem.storeLink' has been added.
//- Property 'BrandItem.id' has been added.

//- Property 'BrandItem.itemType' has been removed.
//- Property 'BrandItem.brightness' has been removed.
//- Property 'BrandItem.saturation' has been removed.
//- Property 'BrandItem.hue' has been removed.
//- Property 'BrandCategory.selectedItem' has been removed.
//- Property 'BrandItem.itemCode' has been removed.
//- Property 'BrandItem.itemID' has been removed.


func migrateZeroToOne(_ migration: Migration) {
 
//BrandCategory
    migration.renamePropertyIfExists(onType: BrandCategory.className(), from: "categoryID", to: "id")
    
//BrandItem
    migration.renamePropertyIfExists(onType: BrandItem.className(), from: "itemID", to: "id")
    
//    migration.renameProperty(onType: BrandItem.className(), from: "itemType", to: "type")
//    migration.renameProperty(onType: BrandItem.className(), from: "itemCode", to: "storeID")
//    migration.renameProperty(onType: BrandItem.className(), from: "storeID", to: "storeLink")
    
}

