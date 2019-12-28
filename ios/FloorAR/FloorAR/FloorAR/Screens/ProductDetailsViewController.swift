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
    
    @IBAction func closeClicked(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }
    
    var product: Product? {
        didSet {
            self.color = self.product?.colors.first ?? nil
        }
    }
    var color: ProductColor?
    
    #if DEBUG
        let flushCache = true
    #else
        let flushCache = false
    #endif
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let script = """
                    function webviewLoaded() {
                        window.setTimeout(function() {
                            webkit.messageHandlers.callbackHandler.postMessage({'command':'loaded'})
                        }, 500)
                    }
                    if (window.attachEvent) {window.attachEvent('onload', webviewLoaded);}
                    else if (window.addEventListener) {window.addEventListener('load', webviewLoaded, false);}
                    else {document.addEventListener('load', webviewLoaded, false);}
                    """
        let userScript = WKUserScript(source: script, injectionTime: .atDocumentStart, forMainFrameOnly: true)
        let link = URL(string:"https://mobile.cambrianar.com/details")!
        let request = URLRequest(url: link, cachePolicy:flushCache ? .reloadIgnoringLocalAndRemoteCacheData : .useProtocolCachePolicy)
        webview.uiDelegate = self
        webview.navigationDelegate = self
        webview.configuration.preferences.javaScriptEnabled = true
        webview.configuration.userContentController.add(self, name: "callbackHandler")
        webview.configuration.userContentController.addUserScript(userScript)
        webview.load(request)
    }
    
    override func viewDidAppear(_ animated: Bool) {
        
    }
    
    func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
        
    }
    
    func productColorChanged(product: Product, color: ProductColor) {
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
}
