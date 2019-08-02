//
//  Section.swift
//  HarmonyApp
//
//  Created by Jaremy Longley on 12/7/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift

struct Section {
    var category: String!
    var expanded: Bool!
    var items: List<BrandItem>!
    
    init(category: String, expanded: Bool, items: List<BrandItem>) {
        self.category = category
        self.expanded = expanded
        self.items = items
    }
    
    func append(_ item: BrandItem) {
        self.items.append(item)
    }
    
    func getItemCount() -> Int {
        if self.expanded || self.items.count < 3 {
            return self.items.count
        } else {
            return 3
        }
    }
}
