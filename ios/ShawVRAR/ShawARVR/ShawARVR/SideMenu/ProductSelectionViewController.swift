//
//  ProductSelectionViewController.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit
import RealmSwift
import SDWebImage

class ProductSwatchCell: UICollectionViewCell {
    @IBOutlet weak var productImage: UIImageView!
    @IBOutlet weak var productLabel: UILabel!
    
    let color = UIColor(red: 51, green: 51, blue: 51)
    var product: Product? {
        didSet {
            self.productLabel.text = product?.name
            self.productLabel.textColor = color
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.sd_setImage(with: product?.thumbnailPath)
        }
    }
    
    var category: ProductCategory? {
        didSet {
            self.productLabel.text = category?.name
            self.productLabel.textColor = color
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.sd_setImage(with: category?.thumbnailPath)
        }
    }
    
    func selected(_ select: Bool, animated:Bool) {
        self.layer.borderWidth = select ? 2.0 : 0.0
        self.layer.borderColor = UIColor.cambrianBlue.cgColor
    }
    
    func getColor() -> UIColor {
        return UIColor.cambrianBlue
    }
}

protocol ProductSelectionDelegate: class {
    func productColorChanged(product: Product, color: ProductColor)
    func productInstallationChanged(product: Product, method: InstallationMethod)
    func sceneLocationChanged(location: SceneLocation)
}

class ProductSelectionViewController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource {
    
    @IBOutlet weak var categoryLabel: UILabel!
    
    @IBOutlet weak var categoryButtons: UIView!
    @IBOutlet weak var categoryButtonsHeight: NSLayoutConstraint!
    @IBOutlet weak var categoryButton0: StandardButton!
    @IBOutlet weak var categoryButton1: StandardButton!
    
    @IBOutlet weak var productCollection: UICollectionView!
    
    var isVR = false
    
    var categories:[ProductCategory]?
    var categorySelectedIndex = 0
    
    var baseCategory:ProductCategory?
    
    var selectedCell:ProductSwatchCell?
    
    var selectedCategory:ProductCategory? {
        get {
            if let categories = self.categories, categorySelectedIndex < categories.count {
                return categories[categorySelectedIndex]
            }
            return baseCategory
        }
    }
    
    var selectedProduct: Product?
    weak open var delegate: ProductSelectionDelegate?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        if let parent = baseCategory {
            self.navigationItem.title = parent.name
            self.navigationItem.titleView = UIView()
            self.categoryLabel.text = parent.name.uppercased()
            self.categoryButtonsHeight.constant = 0
            self.categoryButtons.isHidden = true
        }
        else {
            self.categoryButtonsHeight.constant = 50
            self.categoryButtons.isHidden = false
            self.categories = DataController.sharedInstance.productContext?.objects(ProductCategory.self).sorted(byKeyPath: "orderIndex", ascending: true).filter({$0.parents.count == 0})
        }
        
        refreshData()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        // Hide the navigation bar on the this view controller
        self.navigationController?.setNavigationBarHidden(self.baseCategory == nil, animated: false)
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        self.navigationController?.setNavigationBarHidden(false, animated: false)
    }

    func refreshData() {
        
        guard let categories = self.categories else {
            return
        }
        
        if categories.count > 0 {
            for index in 0...categories.count-1 {
                let button = (index == 0) ? categoryButton0 : categoryButton1
                button?.tag = index
                button?.setTitle(categories[index].name, for: .normal)
                button?.isSelected = (index == self.categorySelectedIndex)
            }
        }
        
        productCollection.reloadData()
    }
    
    @IBAction func categoryClicked(_ sender: Any) {
        if let button = sender as? UIButton {
            categorySelectedIndex = button.tag
            refreshData()
        }
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if let currentCategory = self.selectedCategory {
            return currentCategory.categories.count + currentCategory.products.count
        }
        return 0
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProductSwatchCell", for: indexPath) as? ProductSwatchCell else {
            fatalError("cannot find ProductSwatchCell")
        }
        
        let numCategories = self.selectedCategory!.categories.count
        
        if indexPath.row < numCategories {
            cell.product = nil
            cell.category = self.selectedCategory!.categories[indexPath.row]
        } else {
            cell.category = nil
            cell.product = self.selectedCategory!.products[indexPath.row - numCategories]
        }

        return cell
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        
        selectedCell?.selected(false, animated: true)
        
        let cell = collectionView.cellForItem(at: indexPath) as? ProductSwatchCell
        selectedCell = cell
        cell?.selected(true, animated: true)
        
        if let product = cell?.product {
            self.selectedProduct = product
            performSegue(withIdentifier: "showColors", sender: self)
        } else {
            let storyBoard : UIStoryboard = UIStoryboard(name: "Main", bundle:nil)
            
            if let controller = storyBoard.instantiateViewController(withIdentifier: "ProductSelection") as? ProductSelectionViewController {
                controller.baseCategory = cell?.category
                controller.delegate = self.delegate
                controller.selectedProduct = nil
                self.navigationController?.pushViewController(controller, animated: true)
            }
        }
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
        if let controller = segue.destination as? StyleSelectionViewController {
            controller.selectedProduct = self.selectedProduct
            controller.delegate = self.delegate
            controller.isVR = self.isVR
        }
    }

}
