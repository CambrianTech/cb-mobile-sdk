//
//  BrandCategory_Extended.swift
//  HomeHarmony
//
//  Created by Joel Teply on 11/7/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift
import FirebaseStorage
import AWSS3
import Alamofire

internal protocol BrandCategorySelectionDelegate : NSObjectProtocol {
    func categorySelected(_ sender: AnyObject, category:BrandCategory, indexPath:IndexPath)
}

extension BrandCategory: Drawable {
    
    func needsDarkText() -> Bool {
        if self.color == UIColor(red: 0, green: 0, blue: 0) {
            if let drawable = self.displayItem {
                return drawable.needsDarkText()
            }
        } else {
            // Calculation for darkness or lightness of color
            let calculation = ((self.red * 299) + (self.green * 587) + (self.blue * 114)) / 1000
            if calculation < 125 {
                return false
            } else {
                return true
            }
        }
        return false
    }
    
    var text: String? {
        return self.name
    }
    
    var color:UIColor {
        get {
            return UIColor(red: self.red, green: self.green, blue: self.blue)
        }
    }
    
    var view: UIView? {
        get {
            let black = UIColor(red: 0, green: 0, blue: 0, alpha: 1)
            // if parent category is null, top level so return the display color
            if let parent = self.parentCategory {
                print(self.name)
                let view = UIImageView()
                view.backgroundColor = self.color
                return view
            } else {
                return self.displayItem?.view
            }
        }
    }
    
    func getItemType() -> CBAssetType {
        if let firstItem = self.items.first {
            return firstItem.type
        }
        else if let firstCategory = self.subCategories.first {
            return firstCategory.getItemType()
        }
        return .model
    }
    
    func isEmpty() -> Bool {
        if let firstCategory = self.subCategories.first {
            return firstCategory.isEmpty()
        } else if let item = self.items.first {
            if item.type == .model {
                if (!CBAugmentedModel.canCreate()) {
                    return true;
                }
            }
            return false
        }
        return true
    }
    
    func downloadThumbnails(completed: @escaping () -> Void) {
        if (self.items.last?.hasThumbnail == false) {
            completed()
        } else {
            displayIndeterminateProgress("Downloading Models")
            
            for item in self.items {
                print(item.assetPath)
                let view = item.view
                if(item == self.items.last) {
                    hideProgress()
                    completed()
                }
            }
        }
    }
    
    func isFloor() -> Bool {
        return self.getItemType() == CBAssetType.floor
    }
}








