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
import FBAudienceNetwork

class PhotoViewController: CameraViewController, ProductSelectionDelegate, CBWebViewDelegate, FBAdViewDelegate {
    
    @IBOutlet weak var webview: CBWebView!
    @IBOutlet weak var fbAdBannerContainer:UIView!
    @IBOutlet weak var fbAdViewHeightConstraint: NSLayoutConstraint!
    private var fbAdBanner:FBAdView!
    
    var sceneToLoad:SceneLocation?
    var photoToLoad:UIImage?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        var url = DataSource.visualizerUrl
        if let scene = self.sceneToLoad {
            url = url.appending("scene", value: scene.basePath)
            self.sceneToLoad = nil
        } else {
            url = url.appending("wait", value: "1")
        }

        let request = URLRequest(url: url, cachePolicy:flushCache ? .reloadIgnoringLocalAndRemoteCacheData : .useProtocolCachePolicy)
        
        self.webview.delegate = self
        self.webview.load(request)
        
        self.fbAdBanner = FBAdView(placementID: fbAdBannerPlacementID, adSize: fbAdBannerType, rootViewController:self)
        self.fbAdBanner.delegate = self
        if (UIDevice.current.userInterfaceIdiom == .pad) {
            self.view.addSubview(self.fbAdBanner)
        } else {
            self.fbAdBanner.frame = CGRect(x: 0, y: 0, width: self.fbAdBannerContainer.frame.width, height: fbAdBannerType.size.height)
            self.fbAdBannerContainer.addSubview(self.fbAdBanner)
        }
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        webview.hud.setProgress(0.0, animated: true)
        self.fbAdViewHeightConstraint.constant = 0
        if (adsEnabled) {
            self.fbAdBanner.loadAd()
        }
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        if isBeingDismissed {
            self.webview.unload()
        }
    }
    
    func adViewDidLoad(_ ad: FBAdView) {
        if (UIDevice.current.userInterfaceIdiom == .pad) {
            let adWidth:CGFloat = 728 //leaderboard
            self.fbAdBanner.frame = CGRect(x: 0, y: self.webview.frame.minY, width: adWidth, height: fbAdBannerType.size.height)
        } else {
            self.fbAdViewHeightConstraint.constant = fbAdBannerType.size.height
        }
    }
    
    func adView(_ adView: FBAdView, didFailWithError error: Error) {
        print("Ads failed: \(error.localizedDescription)")
    }
    
    @IBAction func closeClicked(_ sender: Any) {
        self.dismiss(animated: true, completion: nil)
    }
    
    func uploadPhoto() {
        if let photo = self.photoToLoad, let photoData = photo.jpegData(compressionQuality: 90) {
            print("Got photo with \(photoData.count) bytes")
            let base64 = photoData.base64EncodedString(options: [])
            let url = "data:application/jpeg;base64," + base64
            let command = "window.cb.uploadPhotoData('\(url)')"
            
            self.webview.evaluateJavaScript(command, completionHandler: { (result, error) in
                if (error != nil) {
                    print("Command error: could not upload photo")
                    self.webview.hud.dismiss()
                } else {
                    self.didUpload = true
                }
            })
        }
    }
    
    var didUpload = false
    func CBWebViewDidFinishedLoading(_ success: Bool) {
        if (!success) {
            self.webview.hud.dismiss()
            return
        }
    }
    
    func CBWebViewFailedLoad() {
        self.dismiss(animated: true, completion: nil)
    }
    
    func CBWebViewHandleScriptMessage(_ message:Dictionary<String, AnyObject>) {
        if let command = message["command"] as? String {
            if command == "sceneLoaded" {
                self.webview.hud.dismiss()
            } else if command == "notifyLoaded", let component = message["component"] as? String {
                //print("Loaded component " + component)
                if (component == "ImageUpload" && self.photoToLoad != nil) {
                    self.uploadPhoto()
                }
            } else if command == "openImageDialog" {
                //print("Loaded component " + component)
                self.changePhotoClicked()
            }
        }
    }
    
    func CBWebViewShowProgress(show:Bool, isPage:Bool) {
        if (show) {
            self.webview.hud.textLabel.text = "Contacting Server"
            if (isPage) {
                self.webview.hud.setProgress(0.1, animated: true)
            }
            self.webview.hud.show(in: self.view)
        } else if (!isPage)  {
            self.webview.hud.dismiss()
        }
    }
    
    func CBWebViewDisplayProgress(progress:Float, message:String, isPage:Bool) {
        if (!isPage) {
            self.webview.hud.textLabel.text = message
        }
        let factor:Float = (self.photoToLoad == nil) ? 1.0 : 0.5
        self.webview.hud.progress = isPage ? progress * factor : factor + progress * factor
    }
    
    func changePhotoClicked() {
        let optionMenu = UIAlertController(title: nil, message: "What kind of photo do you need?", preferredStyle: .actionSheet)
            
        optionMenu.addAction(UIAlertAction(title: "Take Picture", style: .default, handler:{ (UIAlertAction) in
            self.visualizeImage(true)
        }))
        
        optionMenu.addAction(UIAlertAction(title: "Photo Library", style: .default, handler:{ (UIAlertAction) in
            self.visualizeImage(false)
        }))
        
        optionMenu.addAction(UIAlertAction(title: "Cancel", style: .cancel))
        
        if UI_USER_INTERFACE_IDIOM() == .pad {
            addActionSheetForiPad(actionSheet: optionMenu)
        }
        
        self.present(optionMenu, animated: true, completion: nil)
    }
    
    override func pickedImage(image:UIImage) {
        self.photoToLoad = image
        self.uploadPhoto()
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

