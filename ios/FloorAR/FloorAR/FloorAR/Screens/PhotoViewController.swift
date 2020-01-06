//
//  WebViewController.swift
//  FloorAR
//
//  Created by Joel Teply on 12/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import UIKit
import WebKit
import JGProgressHUD

class PhotoViewController: UIViewController, ProductSelectionDelegate, WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandler {
    @IBOutlet weak var webview: WKWebView!
    
    var sceneToLoad:SceneLocation?
    let hud = JGProgressHUD(style: .dark)
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let scriptUrl = Bundle.main.url(forResource: "visualizer.js", withExtension: nil)!
        let script = try! String(contentsOf: scriptUrl, encoding: .utf8)
        let userScript = WKUserScript(source: script, injectionTime: .atDocumentEnd, forMainFrameOnly: false)
        
        let url = self.sceneToLoad == nil ? DataSource.visualizerUrl : DataSource.visualizerUrl.appending("scene", value: self.sceneToLoad!.basePath)
                
        let request = URLRequest(url: url, cachePolicy:flushCache ? .reloadIgnoringLocalAndRemoteCacheData : .useProtocolCachePolicy)
        webview.uiDelegate = self
        webview.addObserver(self, forKeyPath: "estimatedProgress", options: .new, context: nil)
        webview.navigationDelegate = self
        webview.configuration.preferences.javaScriptEnabled = true
        webview.configuration.userContentController.addUserScript(userScript)
        webview.configuration.userContentController.add(self, name: "callbackHandler")
        webview.load(request)
        
        hud.indicatorView = JGProgressHUDRingIndicatorView()
        hud.textLabel.text = "Contacting Service"
        hud.show(in: self.view)
    }
    
    override func viewWillAppear(_ animated: Bool) {
        hud.setProgress(hud.progress + 0.2, animated: true)
    }
    
    func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
        self.hud.dismiss()
        
        if let _ = sceneToLoad { } else {
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                self.webview.evaluateJavaScript("window.openImageDialog()", completionHandler: { (result, error) in
                    if (error != nil) {print("Command error: Could not open image dialog")}
                })
            }
        }
    }
    
    override func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey : Any]?, context: UnsafeMutableRawPointer?)
    {
        if (keyPath == "estimatedProgress") { // listen to changes and updated view
            hud.setProgress(Float(webview.estimatedProgress), animated: true)
        }
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
                print("Command error: could not load material")
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
    
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
        let body = message.body
        if let dict = body as? Dictionary<String, AnyObject> {
            if let command = dict["command"] as? String {
                if command == "showProgress" {
                    let show = dict["show"] as! Bool
                    if (show) {
                        self.hud.textLabel.text = "Uploading"
                        self.hud.setProgress(0.1, animated: true)
                        self.hud.show(in: self.view)
                    } else {
                        self.hud.dismiss()
                    }
                }
                else if command == "setProgress" {
                    let progress = dict["progress"] as! NSNumber
                    self.hud.textLabel.text = (dict["message"] as! String)
                    self.hud.setProgress(Float(truncating: progress), animated: true)
                }
            }
        }
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

