//
//  DetailsViewController.swift
//  HarmonyApp
//
//  Created by Jaremy Longley on 6/29/17.
//  Copyright © 2017 Cambrian. All rights reserved.
//
import Foundation

internal protocol DetailsDelegate: NSObjectProtocol {
    func itemPressed(_ item: BrandItem)
    func visualizerPressed()
}

class DetailsViewController: UIViewController, sharedFavoritesDelegate {
    
    
    @IBOutlet weak var favoriteButton: FavoritesButton!
    @IBOutlet weak var visualizeButton: UIButton!
    @IBOutlet weak var colorLabel: UILabel!
    
    weak internal var delegate: DetailsDelegate?
    var item:BrandItem? {
        didSet {
            update()
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        
    }
    
    @IBAction func favoriteButtonPressed(_ sender: Any) {
        
    }
    
    @IBAction func visualizeButtonPressed(_ sender: Any) {
        self.delegate?.visualizerPressed()
    }
    
    func update() {
        if let item = self.item {
            self.view.backgroundColor = item.color
            
            favoriteButton.item = item
            
            var text = ""
            text += "\(item.name) - \(item.storeID)"
            
            let paragraphStyle = NSMutableParagraphStyle()
            paragraphStyle.lineSpacing = 5
            
            let attString = NSMutableAttributedString(string: text)
            attString.addAttribute(NSAttributedStringKey.paragraphStyle,
                                   value: paragraphStyle,
                                   range:NSMakeRange(0, attString.length))
            
            self.colorLabel.attributedText = attString
            self.colorLabel.textColor = (item.needsDarkText() ? UIColor.black : UIColor.white)
            self.delegate?.itemPressed(item)
        }
    }
    
    func updateFavorites(_ item: BrandItem) {
        if let item = self.item {
            self.favoriteButton.update(item)
        }
    }
}
