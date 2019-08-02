
//
//  ColorWheel.swift
//  Prestige
//
//  Created by Joel Teply on 6/22/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation
import RealmSwift

internal protocol WheelDelegate : NSObjectProtocol {
    func wheelItemSelected(_ item:BrandItem)
}

class SelectorWheel: UIView, SelectorWheelRingDelegate, UIGestureRecognizerDelegate {
    
    weak internal var delegate: WheelDelegate?
    
    let elementSize = CGSize(width: 120, height: 100)
    var isLandscape = false
    
    var initialRadius:CGFloat = 130
    let elementRadiusClipping:CGFloat = 10
    
    var topRing:Int = 0
    
    var hasSetFirst = false
    
    let baseCategory = BrandCategory()
    
    var selectedItem: BrandItem? {
        didSet {
            if(hasSetFirst) {
                selectItem(selectedItem!)
            }
            hasSetFirst = true
        }
    }
    
    func setupInitialRing() {
        baseCategory.name = "baseCategory"
        
        if let categories = DataController.sharedInstance.brandContext?
            .objects(BrandCategory.self)
            .filter({$0.parentCategory == nil && !$0.isEmpty()}) {
            
            if(categories.count == 1) {
                for category in categories.first!.subCategories {
                    baseCategory.subCategories.append(category)
                }
            } else {
                for category in categories {
                    baseCategory.subCategories.append(category)
                }
            }
        }
        
        addRing(category: baseCategory)
    }
    
    
    func showHide(_ show:Bool, animated:Bool, animationCallback:(()->(Void))? = nil, completion:(()->(Void))? = nil) {
        if show == !self.isHidden {
            return
        }
        let screen = UIScreen.main.bounds
        if (animated) {
            self.isHidden = false
            
            UIView.animate(withDuration: 0.2, delay: 0.0, options: UIViewAnimationOptions(), animations: {
                if(show) {
                    self.frame.origin.y = screen.height - (self.frame.height * 0.65)
                } else {
                    self.frame.origin.y = screen.height
                }
                
                if let handler = animationCallback {
                    handler()
                }
                self.superview?.layoutSubviews()
            })
        } else {
            self.superview?.layoutSubviews()
        }
    }
    
    
    fileprivate var rings:[SelectorWheelRing] = []
    
    func addRing(category:BrandCategory) {
        
        let frame = CGRect(x:0, y:self.frame.height+50, width: self.frame.size.width, height: self.frame.size.height)
        
        let ring = SelectorWheelRing(frame: frame)
        ring.delegate = self
        ring.itemSize = self.elementSize
        ring.row = self.rings.count
        ring.isLandscape = self.isLandscape
        applyPlainShadow(ring, offset: CGSize(width: 0, height: -5))
        
        if self.rings.isEmpty {
            ring.initialSetup = true
        }
        
        self.rings.append(ring)
        self.topRing = rings.count - 1
        self.insertSubview(ring, at: 0)
        
        ring.category = category
        setNeedsLayout()
    }
    
    override func layoutSubviews() {
        if self.rings.isEmpty { return }
        
        //          let innerRadius = self.frame.size.width * 0.35
        //          let outerRadius = innerRadius + self.elementSize.height + 3
        
        self.initialRadius = self.frame.size.width * 0.25
        let radiusIncrement = self.elementSize.height - self.elementRadiusClipping
        var innerRadius = initialRadius
        
        var changed = false
        
        for ring in self.rings {
            if (ring.innerRadius != innerRadius) {
                changed = true
            }
            
            ring.innerRadius = innerRadius
            //               ring.outerRadius = outerRadius
            ring.outerRadius = ring.innerRadius + radiusIncrement + self.elementRadiusClipping
            ring.innerRadiusClipping = self.elementRadiusClipping
            innerRadius += radiusIncrement
            ring.drawBG()
        }
        
        if (changed) {
            ringsChanged()
        }
    }
    
    func updateFavorites() {
        //          if self.rings[topRing].category?.name == "Favorites" {
        //               let favorites = Favorites.sharedInstance.getAsCategory()
        //               print("count \(favorites.items.count)")
        //               if favorites.items.count > 0 {
        //                    self.rings[topRing].category = favorites
        //               } else {
        //                    print("size \(self.baseCategory.subCategories.count)")
        //                    self.baseCategory.subCategories.removeLast()
        //                    print("size \(self.baseCategory.subCategories.count)")
        //                    rings[0].category = self.baseCategory
        //                    self.topRing -= 1
        //                    ringsChanged()
        //               }
        //          } else {
        //               if self.baseCategory.subCategories.filter({ $0.name == "Favorites" }).first == nil {
        //                    self.baseCategory.subCategories.append(Favorites.sharedInstance.getAsCategory())
        //                    rings[0].category = baseCategory
        //               }
        //               else {
        //                    let favorites = Favorites.sharedInstance.getAsCategory()
        //                    if favorites.items.count == 0 {
        //                         self.baseCategory.subCategories.removeLast()
        //                         self.rings[0].collectionView.reloadData()
        //                    }
        //               }
        //          }
    }
    
    func ringsChanged() {
        UIView.animate(withDuration: 0.5, delay: 0.0, options: UIViewAnimationOptions(), animations: {
            self.setNeedsLayout()
            
            var inc = 0
            for ring in self.rings {
                if(inc == self.topRing - 1) {
                    // ring moves below top position
                    ring.frame.origin.y = self.frame.height * 0.6
                } else if (inc == self.topRing) {
                    // ring is in the top ring position
                    ring.frame.origin.y = 0
                } else {
                    // ring moves out of view completely
                    ring.frame.origin.y = self.frame.height + 50
                }
                inc += 1
            }
        })
    }
    
    func selectItem(_ item:BrandItem) {
        if item == selectedItem { return }
        self.topRing = 0
        ringsChanged()
        
        var category = item.parentCategory
        let hierarchy = List<BrandCategory>()
        while category != nil {
            hierarchy.append(category!)
            category = category?.parentCategory
        }
        
        for (index, category) in hierarchy.reversed().enumerated() {
            if let position = self.rings[index].categoryIndex(category) {
                _ = categorySelected(category, ring: rings[index])
                self.rings[index].scrollPosition = position
            }
        }
        
        if let position = self.rings[self.topRing].itemIndex(item) {
            self.selectedItem = item
            self.rings[self.topRing].scrollPosition = position
        }
    }
    
    func categorySelected(_ category: BrandCategory, ring:SelectorWheelRing) -> Bool{
        if ring != rings[topRing] {
            // Detects a ring that is not the top ring selected
            // and does not add a ring based on the cell clicked
            self.topRing -= 1
            ringsChanged()
            return false
        } else {
            //if there is no existing ring above this
            if self.topRing == self.rings.count - 1 {
                addRing(category: category)
            } else {
                self.topRing += 1
                if(category.name == "Favorites") {
                    //self.rings[topRing].category = Favorites.sharedInstance.getAsCategory() //refresh favorites when they're opened
                } else {
                    self.rings[topRing].category = category
                }
                ringsChanged()
            }
            return true
        }
    }
    
    func itemSelected(_ item:BrandItem, row: Int) {
        selectedItem = item
        self.delegate?.wheelItemSelected(item)
    }
}
