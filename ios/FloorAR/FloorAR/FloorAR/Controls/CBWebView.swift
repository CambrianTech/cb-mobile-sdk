//
//  CBWebView.swift
//  FloorAR
//
//  Created by Joel Teply on 1/6/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import UIKit
import WebKit
import JGProgressHUD

@objc protocol CBWebViewDelegate: AnyObject {
    @objc optional func CBWebViewHandleStatusCode(_ status:Int)
    @objc optional func CBWebViewHandleAlert(message:String, completionHandler: () -> Void)
    @objc optional func CBWebViewHandleScriptMessage(_ message:Dictionary<String, AnyObject>)
    @objc optional func CBWebViewDidFinishedLoading(_ success:Bool)
    @objc optional func CBWebViewShowProgress(show:Bool, isPage:Bool)
    @objc optional func CBWebViewDisplayProgress(progress:Float, message:String, isPage:Bool)
}

class CBWebView: WKWebView, WKUIDelegate, WKNavigationDelegate, WKScriptMessageHandler {
    
    let hud = JGProgressHUD(style: .dark)
    weak open var delegate: CBWebViewDelegate?
    var showLoadingIndicator = true
    
    func initialize() {
        let scriptUrl = Bundle.main.url(forResource: "CBWebView.js", withExtension: nil)!
        let script = try! String(contentsOf: scriptUrl, encoding: .utf8)
        let userScript = WKUserScript(source: script, injectionTime: .atDocumentEnd, forMainFrameOnly: false)
        
        self.uiDelegate = self
        self.addObserver(self, forKeyPath: "estimatedProgress", options: .new, context: nil)
        self.navigationDelegate = self
        self.configuration.preferences.javaScriptEnabled = true
        self.configuration.userContentController.addUserScript(userScript)
        self.configuration.userContentController.add(self, name: "callbackHandler")
        
        hud.indicatorView = JGProgressHUDRingIndicatorView()
        hud.textLabel.text = "Loading"
    }
    
    override init(frame: CGRect, configuration: WKWebViewConfiguration) {
        super.init(frame: frame, configuration: configuration)
        initialize()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        initialize()
    }
    
    @discardableResult override open func load(_ request: URLRequest) -> WKNavigation? {
        if (showLoadingIndicator) {
            displayProgress(show: true, isPage:true)
        }
        return super.load(request)
    }
    
    func unload() {
        let request = URLRequest(url: URL(string: "about:blank")!)
        self.load(request)
    }
    
    override func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey : Any]?, context: UnsafeMutableRawPointer?)
    {
        if (keyPath == "estimatedProgress") { // listen to changes and updated view
            let progress = Float(self.estimatedProgress)
            
            updateProgress(progress: progress, message: "Loading", isPage: true)
            
            if (progress == 1.0) {
                //shut off
                displayProgress(show: false, isPage:true)
                if let title = self.title, title.count == 0 {
                    self.handleStatusCode(404)
                    if let callback = self.delegate?.CBWebViewDidFinishedLoading {
                        callback(false)
                    }
                }
            }
        }
    }
    
    func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
        displayProgress(show: false, isPage:true)
    }
    
    func webView(_ webView: WKWebView, decidePolicyFor navigationResponse: WKNavigationResponse, decisionHandler: @escaping (WKNavigationResponsePolicy) -> Void) {

        if let response = navigationResponse.response as? HTTPURLResponse {
            let statusCode = response.statusCode
            handleStatusCode(statusCode)
            
            //maybe more logic:
            if statusCode == 200 {
                //OK
                decisionHandler(.allow)
            } else {
                decisionHandler(.cancel)
                if let callback = self.delegate?.CBWebViewDidFinishedLoading {
                    callback(false)
                }
            }
        }
    }
    
    func handleStatusCode(_ status:Int) {
        displayProgress(show: false, isPage:true)
        if let callback = self.delegate?.CBWebViewHandleStatusCode {
            callback(status)
        }
    }
    
    func webView(_ webView: WKWebView, runJavaScriptAlertPanelWithMessage message: String, initiatedByFrame frame: WKFrameInfo, completionHandler: () -> Void) {
        if let callback = self.delegate?.CBWebViewHandleAlert {
            callback(message, completionHandler)
        } else if let vc = self.parentViewController {
            let alert = UIAlertController(title: "Alert", message: message, preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
            vc.present(alert, animated: true)
            completionHandler()
        }
    }
    
    func displayProgress(show:Bool, isPage:Bool) {
        if let callback = self.delegate?.CBWebViewShowProgress {
            callback(show, isPage);
        } else {
            if (show) {
                self.hud.show(in: self)
            } else {
                self.hud.dismiss()
            }
        }
    }
    
    func updateProgress(progress:Float, message:String, isPage:Bool) {
        if let callback = self.delegate?.CBWebViewDisplayProgress {
            callback(progress, message, isPage)
        } else {
            self.hud.textLabel.text = message
            self.hud.progress = progress
        }
    }
    
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
        let body = message.body
        if let dict = body as? Dictionary<String, AnyObject> {
            if let callback = self.delegate?.CBWebViewHandleScriptMessage {
                callback(dict)
            }
            
            if let command = dict["command"] as? String {
                //print("Got command \(command)")
                if command == "loaded" {
                    if let callback = self.delegate?.CBWebViewDidFinishedLoading {
                        callback(true)
                    }
                }
                else if command == "showProgress" {
                    let show = dict["show"] as! Bool
                    
                    if let callback = self.delegate?.CBWebViewShowProgress {
                        callback(show, false)
                    } else {
                        if (show) {
                            self.hud.textLabel.text = "Uploading"
                            self.hud.setProgress(0.1, animated: true)
                        }
                        
                        displayProgress(show: show, isPage:false)
                    }
                }
                else if command == "setProgress" {
                    let progress = Float(truncating: dict["progress"] as! NSNumber)
                    let message = dict["message"] as! String
                    updateProgress(progress: progress, message: message, isPage: false)
                }
            }
        }
        
        
    }
}
