//
//  ProductDetailsViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/27/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import WebKit

class ProductDetailsViewController: UIViewController, ProductSelectionDelegate, WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandler {
    @IBOutlet weak var webview: WKWebView!
    @IBOutlet weak var productLabel: UILabel?
    
    @IBAction func closeClicked(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }
    
    private var colorSelector:ProductSelectionView?
    
    var category: ProductCategory?
    
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

        //reading
        if let scriptUrl = Bundle.main.url(forResource: "product-details.js", withExtension: nil) {
            do {
                let script = try String(contentsOf: scriptUrl, encoding: .utf8)
                let userScript = WKUserScript(source: script, injectionTime: .atDocumentEnd, forMainFrameOnly: false)

                let request = URLRequest(url: DataSource.productDetailsUrl)
                webview.uiDelegate = self
                webview.navigationDelegate = self
                webview.configuration.preferences.javaScriptEnabled = true
                webview.configuration.userContentController.add(self, name: "callbackHandler")
                webview.configuration.userContentController.addUserScript(userScript)
                webview.load(request)
            }
            catch {}
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
        guard let category = self.category else {
            return
        }
        self.product = product
        self.color = color
        
        var material = Dictionary<String,Any>()
        material["ppi"] = product.ppi;
        material["diffuseUrl"] = color.defaultVariation.remoteDiffusePath?.absoluteString;
        material["normalsUrl"] = color.defaultVariation.remoteNormalPath?.absoluteString ?? nil
        material["specularUrl"] = color.defaultVariation.remoteRoughnessPath?.absoluteString ?? nil;
        
        let detailsJson = "{'name':'\(color.code)', 'category':\(category.jsonString), 'product':\(product.jsonString), 'color':\(color.jsonString)}"
        let command = "if (setProductDetails) setProductDetails(\(material.jsonString), \(detailsJson))"
        self.webview.evaluateJavaScript(command, completionHandler: { (result, error) in
            if (error != nil) {
                print("Command error")
            } else {
                print("Command worked!")
            }
        })
    }
    
    func webView(_ webView: WKWebView, runJavaScriptAlertPanelWithMessage message: String, initiatedByFrame frame: WKFrameInfo, completionHandler: () -> Void) {
        let alert = UIAlertController(title: "Alert", message: message, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        self.present(alert, animated: true)
        
        completionHandler()
    }
    
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
        let body = message.body
        if let dict = body as? Dictionary<String, AnyObject> {
            if let command = dict["command"] as? String {
                if command == "loaded", let product = self.product, let color = self.color {
                    productColorChanged(product: product, color: color)
                }
            }
        }
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
