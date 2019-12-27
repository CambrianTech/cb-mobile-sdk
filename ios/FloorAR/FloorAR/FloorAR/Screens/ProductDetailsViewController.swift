//
//  ProductDetailsViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/27/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import WebKit

class ProductDetailsViewController: UIViewController, ProductSelectionDelegate, WKUIDelegate, WKNavigationDelegate {
    @IBOutlet weak var webview: WKWebView!
    
    var product: Product?
    var color: ProductColor?
    
    #if DEBUG
        let flushCache = true
    #else
        let flushCache = false
    #endif
    
    override func viewDidLoad() {
        super.viewDidLoad()
                
        let link = URL(string:"https://mobile.cambrianar.com/details")!
        let request = URLRequest(url: link, cachePolicy:flushCache ? .reloadIgnoringLocalAndRemoteCacheData : .useProtocolCachePolicy)
        webview.uiDelegate = self
        webview.navigationDelegate = self
        webview.configuration.preferences.javaScriptEnabled = true
        webview.load(request)
    }
    
    func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
        if let product = self.product, let color = self.color {
            productColorChanged(product: product, color: color)
        }
    }
    
    func productColorChanged(product: Product, color: ProductColor) {
        //self.webView.evaluateJavaScript("window.webkit.messageHandlers.iosListener.postMessage('test');", completionHandler: { (result, err) in
        var material = Dictionary<String,Any>()
        material["ppi"] = product.ppi;
        material["diffuseUrl"] = color.defaultVariation.remoteDiffusePath?.absoluteString;
        material["normalsUrl"] = color.defaultVariation.remoteNormalPath?.absoluteString ?? nil
        material["specularUrl"] = color.defaultVariation.remoteRoughnessPath?.absoluteString ?? nil;
        
        let command = "window.cb.setMaterial(\(material.jsonString))"
        self.webview.evaluateJavaScript(command, completionHandler: { (result, error) in
            if (error != nil) {
                print("Command error")
            }
        })
    }
}




