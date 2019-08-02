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

class ProductStyleSwatchCell: UICollectionViewCell {
    @IBOutlet weak var productImage: UIImageView!
    @IBOutlet weak var productLabel: UILabel!
    
    let textColor = UIColor.white
    var color: ProductColor? {
        didSet {
            self.productLabel.text = color?.name
            self.productLabel.textColor = textColor
            self.productImage.sd_setImage(with: color?.thumbnailPath)
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

class InstallationMethodCell: UICollectionViewCell {
    @IBOutlet weak var productImage: UIImageView!
    @IBOutlet weak var productLabel: UILabel!
    
    let color = UIColor(red: 51, green: 51, blue: 51)
    var installationMethod: InstallationMethod? {
        didSet {
            self.productLabel.text = installationMethod?.name
            self.productLabel.textColor = color
            self.productLabel.font = UIFont.systemFont(ofSize: 15.0, weight: .medium)
            self.productImage.image = installationMethod?.thumbnailImage
            
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

class SceneLocationCell: UICollectionViewCell {
    @IBOutlet weak var sceneImage: UIImageView!
    
    let textColor = UIColor.white
    var location: SceneLocation? {
        didSet {
            self.sceneImage.image = location?.thumbnailImage
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

class StyleSelectionViewController: UIViewController, UICollectionViewDelegate, UICollectionViewDataSource {
    
    @IBOutlet weak var productLabel: UILabel!
    @IBOutlet weak var styleCollection: UICollectionView!
    @IBOutlet weak var installationCollection: UICollectionView!
    @IBOutlet weak var sceneCollection: UICollectionView!
    @IBOutlet weak var sceneCollectionView: UIView!
    @IBOutlet weak var sceneCollectionViewHeight: NSLayoutConstraint!
    
    weak open var delegate: ProductSelectionDelegate?
    var isVR = false
    
    var selectedProduct: Product?
    var selectedProductColor: ProductColor?
    var selectedInstallationMethod: InstallationMethod?
    
    var selectedColorCell:ProductStyleSwatchCell?
    var selectedMethodCell:InstallationMethodCell?
    var selectedLocationCell:SceneLocationCell?
    
    var sceneLocations:Results<SceneLocation>?
    
    override func viewDidLoad() {
        super.viewDidLoad()

        self.styleCollection.allowsMultipleSelection = false
        self.installationCollection.allowsMultipleSelection = false
        
        self.sceneLocations = DataController.sharedInstance.productContext?.objects(SceneLocation.self).sorted(byKeyPath: "orderIndex", ascending: true)
    }

    override func viewWillAppear(_ animated: Bool) {
        reloadData()
        
        self.sceneCollectionViewHeight.constant = isVR ? 100 : 0
        //first time loading, send default values
        //sendColorChanged()
        //sendMethodChanged()
    }
    
    private func reloadData() {
        self.productLabel.text = selectedProduct?.name.uppercased()
        
        //select first if nothing chosen yet
        if self.selectedProductColor == nil {
            self.selectedProductColor = self.selectedProduct?.colors.first
        }
        
        if self.selectedInstallationMethod == nil {
            self.selectedInstallationMethod = self.selectedProduct?.installationMethods.first
        }
        
        styleCollection.reloadData()
        installationCollection.reloadData()
    }
    
    func collectionView(_ collectionView: UICollectionView, numberOfItemsInSection section: Int) -> Int {
        if (collectionView == self.styleCollection) {
            return self.selectedProduct?.colors.count ?? 0
        } else if (collectionView == self.installationCollection) {
            return self.selectedProduct?.installationMethods.count ?? 0
        } else if (collectionView == self.sceneCollection) {
            return self.sceneLocations?.count ?? 0
        }
        return 0
    }
    
    func collectionView(_ collectionView: UICollectionView, cellForItemAt indexPath: IndexPath) -> UICollectionViewCell {
        if (collectionView == self.styleCollection) {
            guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "ProductStyleSwatchCell", for: indexPath) as? ProductStyleSwatchCell else {
                fatalError("cannot find ProductStyleSwatchCell")
            }
            cell.color = self.selectedProduct!.colors[indexPath.row]
            
            return cell
        } else if (collectionView == self.installationCollection) {
            guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "InstallationMethodCell", for: indexPath) as? InstallationMethodCell else {
                fatalError("cannot find InstallationMethodCell")
            }
            cell.installationMethod = self.selectedProduct!.installationMethods[indexPath.row]
            return cell
        }
        else if (collectionView == self.sceneCollection) {
            guard let cell = collectionView.dequeueReusableCell(withReuseIdentifier: "SceneLocationCell", for: indexPath) as? SceneLocationCell else {
                fatalError("cannot find SceneLocationCell")
            }
            cell.location = self.sceneLocations![indexPath.row]
            return cell
        }
        return UICollectionViewCell()
    }
    
    func collectionView(_ collectionView: UICollectionView, didSelectItemAt indexPath: IndexPath) {
        if let cell = collectionView.cellForItem(at: indexPath) as? ProductStyleSwatchCell {
            selectedColorCell?.selected(false, animated: true)
            selectedColorCell = cell
            cell.selected(true, animated: true)
            self.selectedProductColor = cell.color
            sendColorChanged()
        } else if let cell = collectionView.cellForItem(at: indexPath) as? InstallationMethodCell {
            selectedMethodCell?.selected(false, animated: true)
            selectedMethodCell = cell
            cell.selected(true, animated: true)
            self.selectedInstallationMethod = cell.installationMethod
            sendMethodChanged()
        }
        else if let cell = collectionView.cellForItem(at: indexPath) as? SceneLocationCell {
            selectedLocationCell?.selected(false, animated: true)
            selectedLocationCell = cell
            cell.selected(true, animated: true)
            if let location = cell.location {
                self.delegate?.sceneLocationChanged(location: location)
            }
        }
    }
    
    private func sendColorChanged() {
        guard let product = self.selectedProduct,
            let color = self.selectedProductColor else {
                return
        }
        
        self.delegate?.productColorChanged(product: product,
                                           color: color)
    }
    
    private func sendMethodChanged() {
        guard let product = self.selectedProduct,
            let method = self.selectedInstallationMethod else {
                return
        }
        
        self.delegate?.productInstallationChanged(product: product,
                                                  method: method)
    }

    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if let controller = segue.destination as? SwatchViewController {
            controller.selectedColor = self.selectedProductColor
        } else if let controller = segue.destination as? SpecificationsViewController {
            controller.selectedProduct = self.selectedProduct
        }
    }

}
