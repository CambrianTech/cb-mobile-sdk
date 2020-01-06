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

class PhotoViewController: UIViewController, ProductSelectionDelegate, CBWebViewDelegate {
    
    @IBOutlet weak var webview: CBWebView!
    
    var sceneToLoad:SceneLocation?
    var photoToLoad:UIImage?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        var url = DataSource.visualizerUrl
        if let scene = self.sceneToLoad {
            url = url.appending("scene", value: scene.basePath)
        } else {
            url = url.appending("wait", value: "1")
        }

        let request = URLRequest(url: url, cachePolicy:flushCache ? .reloadIgnoringLocalAndRemoteCacheData : .useProtocolCachePolicy)
        self.webview.delegate = self
        self.webview.load(request)
    }
    
    override func viewWillAppear(_ animated: Bool) {
        webview.hud.setProgress(0.0, animated: true)
        
        
//        if let _ = sceneToLoad { } else {
//            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
//                self.webview.evaluateJavaScript("window.openImageDialog()", completionHandler: { (result, error) in
//                    if (error != nil) {print("Command error: Could not open image dialog")}
//                })
//            }
//        }
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        if isBeingDismissed {
            self.webview.unload()
        }
    }
    
    func CBWebViewHandleStatusCode(_ status: Int) {
        
    }
    
    func CBWebViewHandleAlert(message: String, completionHandler: () -> Void) {
        let alert = UIAlertController(title: "Alert", message: message, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        self.present(alert, animated: true)
        completionHandler()
    }
    
    func CBWebViewHandleScriptMessage(_ message: WKScriptMessage) {
        
    }
    
    func CBWebViewDidFinishedLoading(_ success: Bool) {
        if (!success) {
            return
        }
        
        if let photo = self.photoToLoad, let photoData = photo.jpegData(compressionQuality: 90) {
            print("Got photo with \(photoData.count) bytes")
            
        }
        
//        if let _ = sceneToLoad { } else {
//            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
//                self.webview.evaluateJavaScript("window.openImageDialog()", completionHandler: { (result, error) in
//                    if (error != nil) {print("Command error: Could not open image dialog")}
//                })
//            }
//        }
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
    
    override func present(_ viewControllerToPresent: UIViewController, animated flag: Bool, completion: (() -> Void)? = nil) {
        setUIDocumentMenuViewControllerSoureViewsIfNeeded(viewControllerToPresent)
        super.present(viewControllerToPresent, animated: flag, completion: completion)
    }

    func setUIDocumentMenuViewControllerSoureViewsIfNeeded(_ viewControllerToPresent: UIViewController) {
        viewControllerToPresent.popoverPresentationController?.sourceView = webview
        viewControllerToPresent.popoverPresentationController?.sourceRect = CGRect(x: webview.center.x, y: webview.frame.maxY - 20, width: 1, height: 1)
    }
}

