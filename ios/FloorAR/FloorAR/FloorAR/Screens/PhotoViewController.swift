//
//  WebViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import WebKit

class PhotoViewController: UIViewController, ProductSelectionDelegate, WKUIDelegate, WKNavigationDelegate {
    @IBOutlet weak var webview: WKWebView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
                
        let link = URL(string:"https://mobile.cambrianar.com")!
        let request = URLRequest(url: link, cachePolicy:flushCache ? .reloadIgnoringLocalAndRemoteCacheData : .useProtocolCachePolicy)
        webview.uiDelegate = self
        webview.navigationDelegate = self
        webview.configuration.preferences.javaScriptEnabled = true
        webview.load(request)
    }
    
    func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {        
        self.webview.evaluateJavaScript("window.openImageDialog()", completionHandler: { (result, error) in
            if (error != nil) {
                print("Command error")
            }
        })
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
    
    private var categorySelector:ProductSelectionView?
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        if segue.identifier == "product-navigation" {
            if let categorySelector = segue.destination as? ProductSelectionView {
                categorySelector.delegate = self
                self.categorySelector = categorySelector
            }
        }
    }
    
    func webView(_ webView: WKWebView, runJavaScriptAlertPanelWithMessage message: String, initiatedByFrame frame: WKFrameInfo, completionHandler: () -> Void) {
        let alert = UIAlertController(title: "Alert", message: message, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        self.present(alert, animated: true)
        
        completionHandler()
    }
    
//    override func present(_ viewControllerToPresent: UIViewController, animated flag: Bool, completion: (() -> Void)? = nil) {
//        let documentPicker = UIDocumentPickerViewController(documentTypes: [String(kUTTypeJPEG), String(kUTTypePNG)], in: .import)
//        super.present(documentPicker, animated: flag, completion: completion)
//    }
    
    override func present(_ viewControllerToPresent: UIViewController, animated flag: Bool, completion: (() -> Void)? = nil) {
        setUIDocumentMenuViewControllerSoureViewsIfNeeded(viewControllerToPresent)
        super.present(viewControllerToPresent, animated: flag, completion: completion)
    }

    func setUIDocumentMenuViewControllerSoureViewsIfNeeded(_ viewControllerToPresent: UIViewController) {
        viewControllerToPresent.popoverPresentationController?.sourceView = webview
        viewControllerToPresent.popoverPresentationController?.sourceRect = CGRect(x: webview.center.x, y: webview.frame.maxY - 20, width: 1, height: 1)
    }
}

