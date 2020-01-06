//
//  ProductDetailsViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/27/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import WebKit
import JGProgressHUD

class ProductDetailsViewController: UIViewController, ProductSelectionDelegate, CBWebViewDelegate {
    @IBOutlet weak var webview: CBWebView!
    @IBOutlet weak var productLabel: UILabel?
    
    @IBAction func closeClicked(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }
    
    private var colorSelector:ProductSelectionView?
        
    var product: Product? {
        didSet {
            self.color = self.product?.colors.first ?? nil
        }
    }
    var color: ProductColor? {
        didSet {
            setProductInfo()
        }
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()

        let scriptUrl = Bundle.main.url(forResource: "product-details.js", withExtension: nil)!
        let script = try! String(contentsOf: scriptUrl, encoding: .utf8)
        let userScript = WKUserScript(source: script, injectionTime: .atDocumentEnd, forMainFrameOnly: false)

        let request = URLRequest(url: DataSource.productDetailsUrl)
        webview.delegate = self
        webview.configuration.userContentController.addUserScript(userScript)
        webview.load(request)
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        if isBeingDismissed {
            webview.unload()
        }
    }
    
    func CBWebViewHandleStatusCode(_ status: Int) {
        
    }
    
    func CBWebViewHandleAlert(message: String, completionHandler: () -> Void) {
        
    }
    
    func CBWebViewHandleScriptMessage(_ message: WKScriptMessage) {
        
    }
    
    func CBWebViewDidFinishedLoading(_ success: Bool) {
        if success, let product = self.product, let color = self.color {
            productColorChanged(product: product, color: color)
        }
    }
    
    override func viewWillAppear(_ animated: Bool) {
        if let selector = self.colorSelector {
            selector.selectedColor = color
            selector.reloadSwatches()
        }
        setProductInfo()
    }
    
    func setProductInfo() {
        if let product = self.product, let color = self.color {
            if let label = self.productLabel  {
                label.text = "\(product.name) - \(color.name)"
            }
        }
    }
    
    func productColorChanged(product: Product, color: ProductColor) {
        self.product = product
        self.color = color
        
        var material = Dictionary<String,Any>()
        material["ppi"] = product.ppi;
        material["diffuseUrl"] = color.defaultVariation.remoteDiffusePath?.absoluteString;
        material["normalsUrl"] = color.defaultVariation.remoteNormalPath?.absoluteString ?? nil
        material["specularUrl"] = color.defaultVariation.remoteRoughnessPath?.absoluteString ?? nil;
        
        let detailsJson = "{'name':'\(color.code)', 'category':\(product.category.jsonString), 'product':\(product.jsonString), 'color':\(color.jsonString)}"
        let command = "if (setProductDetails) setProductDetails(\(material.jsonString), \(detailsJson))"
        self.webview.evaluateJavaScript(command, completionHandler: { (result, error) in
            if (error != nil) {
                print("Command error")
            }
        })
    }
    
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "product-navigation" {
            if let selector = segue.destination as? ProductSelectionView {
                selector.delegate = self
                selector.shouldShowHistory = false
                self.colorSelector = selector
            }
        }
    }
}
