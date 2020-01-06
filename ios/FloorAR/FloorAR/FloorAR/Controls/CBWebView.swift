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

protocol CBWebViewDelegate : NSObjectProtocol {
    func CBWebViewHandleStatusCode(_ status:Int)
    func CBWebViewHandleAlert(message:String, completionHandler: () -> Void)
    func CBWebViewHandleScriptMessage(_ message:WKScriptMessage)
    func CBWebViewDidFinishedLoading(_ success:Bool)
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
            hud.show(in: self)
        }
        return super.load(request)
    }
    
    override func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey : Any]?, context: UnsafeMutableRawPointer?)
    {
        if (keyPath == "estimatedProgress") { // listen to changes and updated view
            let progress = Float(self.estimatedProgress)
            
            hud.setProgress(progress, animated: true)
            if (progress == 1.0) {
                self.hud.dismiss()
                if let title = self.title, title.count == 0 {
                    self.handleStatusCode(404)
                    self.delegate?.CBWebViewDidFinishedLoading(false)
                }
            }
        }
    }
    
    func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
        self.hud.dismiss()
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
                self.delegate?.CBWebViewDidFinishedLoading(false)
            }
        }
    }
    
    func handleStatusCode(_ status:Int) {
        self.hud.dismiss()
        self.delegate?.CBWebViewHandleStatusCode(status)
    }
    
    func webView(_ webView: WKWebView, runJavaScriptAlertPanelWithMessage message: String, initiatedByFrame frame: WKFrameInfo, completionHandler: () -> Void) {
        self.delegate?.CBWebViewHandleAlert(message: message, completionHandler: completionHandler)
    }
    
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
        let body = message.body
        if let dict = body as? Dictionary<String, AnyObject> {
            if let command = dict["command"] as? String {
                if command == "loaded" {
                    self.delegate?.CBWebViewDidFinishedLoading(true)
                }
                else if command == "showProgress" {
                    let show = dict["show"] as! Bool
                    if (show) {
                        self.hud.textLabel.text = "Uploading"
                        self.hud.setProgress(0.1, animated: true)
                        self.hud.show(in: self)
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
        
        self.delegate?.CBWebViewHandleScriptMessage(message)
    }
}
