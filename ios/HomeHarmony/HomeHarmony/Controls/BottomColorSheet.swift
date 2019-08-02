//
//  ColorDetailsPopup.swift
//  Prestige
//
//  Created by Joel Teply on 8/16/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

class BottomColorSheet : UIViewController, sharedFavoritesDelegate {
    
    @IBOutlet weak var headerTitle: UILabel!
    @IBOutlet weak var itemView: UIImageView!
    @IBOutlet weak var itemInfoLabel: UILabel!
    @IBOutlet weak var hideContainer: UIView!
    @IBOutlet weak var brandImage: UIImageView!
    @IBOutlet weak var favoritesButton: FavoritesButton!
    
    @IBOutlet weak var buyButton: LandingPageButton!
        
    var item:BrandItem? {
        didSet {
            update()
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        buyButton.backgroundColor = appColor
        
        let tap = UITapGestureRecognizer(target: self, action: #selector(self.hidePressed(_:)))
        hideContainer.isUserInteractionEnabled = true
        hideContainer.addGestureRecognizer(tap)
        
        let swipe = UISwipeGestureRecognizer(target: self, action: #selector(self.hidePressed(_:)))
        swipe.direction = .down
        self.view.addGestureRecognizer(swipe)
        
        applyPlainShadow(self.view, offset:CGSize(width:0, height:-3))
        
        if (getTargetName() == "Builder") {
            self.favoritesButton.isHidden = true
            self.favoritesButton.isEnabled = false
        }
    }
    
    
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        
        self.view.frame.origin.y = self.view.superview!.frame.height
        self.view.frame.size.width = self.view.superview!.frame.width
        self.view.frame.size.height = 420
    }
    
    
    @IBAction func hidePressed(_ sender: AnyObject) {
        self.show(false)
    }
    
    @IBAction func purchasePressed(_ sender: AnyObject) {
        self.show(false)
        self.item?.gotoStore()
    }
    
    @IBAction func favoritePressed(_ sender: Any) {
        
    }
    
    func update() {
        if let item = self.item {
            self.itemView.backgroundColor = item.color
            if(item.hasThumbnail) {
                self.itemView.kf.setImage(with: item.getDiffusePath())
            } else {
                self.itemView.kf.setImage(with: nil)
            }
            
            buyButton.isHidden = (item.storeLink == "")
            favoritesButton.item = item
            
            var text = ""
            text += "Brand: \(item.getRootCategory().name)\n"
            text += "Name: \(item.name)\n"
            text += "Product ID: \(item.storeID)"
            
            let paragraphStyle = NSMutableParagraphStyle()
            paragraphStyle.lineSpacing = 10
            
            let attrString = NSMutableAttributedString(string: text)
            attrString.addAttribute(NSAttributedStringKey.paragraphStyle, value:paragraphStyle, range:NSMakeRange(0, attrString.length))
            
            headerTitle.text = item.name
            
            self.itemInfoLabel.attributedText = attrString
            
            if let image = UIImage(named: item.getRootCategory().name) {
                brandImage.image = image
            }
        }
    }
    
    func show(_ show:Bool) {
        self.view.isHidden = false
        var finalPos:CGFloat = 0
        
        if(show) {
            finalPos = self.view.superview!.frame.height - self.view.frame.height
        } else {
            finalPos = self.view.superview!.frame.height
        }
        
        //print("bottom sheet shown: \(show)")
        
        UIView.animate(withDuration: 0.3, delay: 0.0, options: UIViewAnimationOptions(), animations: {
            self.view.frame.origin.y = finalPos
            self.view.layoutSubviews()
        }, completion: { finished in
            //print(!show)
            self.view.isHidden = !show
        })
        
    }
    
    func updateFavorites(_ item: BrandItem) {
        if let item = self.item {
            self.favoritesButton.update(item)
        }
    }

}
