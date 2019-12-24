//
//  WebViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import WebKit

class WebViewController: UIViewController, ProductSelectionDelegate, WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandler {
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
        
    }

    @IBOutlet weak var webview: WKWebView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
                
        let link = URL(string:"https://mobile.cambrianar.com")!
        let request = URLRequest(url: link)
        webview.uiDelegate = self
        webview.configuration.preferences.javaScriptEnabled = true
        webview.load(request)
    }
    
    func productColorChanged(product: Product, color: ProductColor) {
        //self.webView.evaluateJavaScript("window.webkit.messageHandlers.iosListener.postMessage('test');", completionHandler: { (result, err) in

        self.webview.evaluateJavaScript("window.test('\(product.name)')", completionHandler: { (result, error) in
            if let error = error {
                print("Error: \(error.localizedDescription)")
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
}
