 //
 //  ProjectProperties.swift
 //  HomeDecoratorApp
 //
 //  Created by Joel Teply on 11/23/15.
 //  Copyright © 2015 Joel Teply. All rights reserved.
 //
 
 import Foundation
 import Alamofire
 import SwiftyJSON
 import Kingfisher
 import Branch
 import Zip
 import FirebaseStorage
 import Social
 import JGProgressHUD
 
 //globals
 private let s3BucketName = "sample-image-packages"
 let sqFfPerExterior:Float = 400
 let sqFfPerInterior:Float = 250
 let doorSizeSqFt:Float = 20
 let windowSizeSqFt:Float = 15
 
 let textAttributes = [NSAttributedStringKey.foregroundColor:UIColor.white]
 
 
 func getStoreURL(_ itemID:String) -> URL {
    return URL(string:"https://www.amazon.com/gp/product/\(itemID)?ie=UTF8&tag=prestige-ios-20")!
 }
 
 func leftChevronImage() -> UIImage? {
    if let image = UIImage(named:"ic_chevron_left") {
        return image.withRenderingMode(UIImageRenderingMode.alwaysTemplate)
    }
    return nil
 }
 
 func rightChevronImage() -> UIImage? {
    if let image = UIImage(named:"ic_chevron_right") {
        return image.withRenderingMode(UIImageRenderingMode.alwaysTemplate)
    }
    return nil
 }
 
 func distanceBetweenColors(_ color1:UIColor, color2:UIColor) -> CGFloat {
    let rgba1 = colorRGBA(color1)
    let rgba2 = colorRGBA(color2)
    
    //because swift sucks at compiling
    let powR = 2 * pow(rgba1[0] - rgba2[0], 2)
    let powG = 4 * pow(rgba1[1] - rgba2[1], 2)
    let powB = 3 * pow(rgba1[2] - rgba2[2], 2)
    
    return sqrt(powR + powG + powB)
 }
 
 func colorRGBA(_ color:UIColor) -> [CGFloat] {
    var fRed : CGFloat = 0
    var fGreen : CGFloat = 0
    var fBlue : CGFloat = 0
    var fAlpha: CGFloat = 0
    
    print(color.getRed(&fRed, green: &fGreen, blue: &fBlue, alpha: &fAlpha))
    color.getRed(&fRed, green: &fGreen, blue: &fBlue, alpha: &fAlpha);
    
    return [fRed, fGreen, fBlue, fAlpha]
 }
 
 
 func ensureDirectoryExists(_ path:String) -> Bool {
    if !FileManager.default.fileExists(atPath: path) {
        do {
            try FileManager.default.createDirectory(atPath: path, withIntermediateDirectories: true, attributes: nil)
        } catch {
            print(error)
            return false
        }
    }
    return true
 }
 
 func initiateFileDownload(_ remotePath:URL, savePath:URL,
                           overwriteFiles:Bool,
                           progressCallback:((_ progress: Progress) -> Void)? = nil,
                           completionCallback:@escaping ((_ success: Bool) -> Void)) {
    
    if !overwriteFiles && FileManager.default.fileExists(atPath: savePath.path) {
        completionCallback(true)
        return
    }
    
    let destination: DownloadRequest.DownloadFileDestination = { _, _ in
        return (savePath, [.removePreviousFile, .createIntermediateDirectories])
    }
    
    Alamofire.download(remotePath, to: destination)
        
        .downloadProgress { progress in
            if let pc = progressCallback {
                pc(progress)
            }
        }
        .response { response in
            
            var success = false
            if let error = response.error {
                print("Download error: \(error.localizedDescription)")
            } else if let finalPath = response.destinationURL?.path {
                success = FileManager.default.fileExists(atPath: finalPath)
            }
            
            DispatchQueue.main.async {
                completionCallback(success)
            }
            
    }
    
 }
 
 
 
 let progressHUD = JGProgressHUD(style: JGProgressHUDStyle.dark)
 var displayingHUD = false
 
 func displayMessage(_ message:String, isSuccess:Bool=false) {
    DispatchQueue.main.async {
        guard let keyWindow = UIApplication.shared.keyWindow else {
            return
        }
        
        if !displayingHUD && !(progressHUD.isVisible) {
            progressHUD.textLabel.text = message
            progressHUD.indicatorView = isSuccess ? JGProgressHUDSuccessIndicatorView() : JGProgressHUDIndicatorView()
            progressHUD.animation = JGProgressHUDFadeZoomAnimation()
        }
        
        progressHUD.show(in: keyWindow, animated: true)
        progressHUD.dismiss(afterDelay: 1.0, animated: true)
    }
 }
 
 func displayProgress(_ message:String, progress:Double) {
    DispatchQueue.main.async {
        guard let keyWindow = UIApplication.shared.keyWindow else {
            return
        }
        
        if !displayingHUD && !(progressHUD.isVisible) {
            displayingHUD = true
            progressHUD.textLabel.text = message
            progressHUD.indicatorView = JGProgressHUDRingIndicatorView()
            progressHUD.animation = JGProgressHUDFadeZoomAnimation()
            
            let delayTime = DispatchTime.now() + Double(Int64(0.5 * Double(NSEC_PER_SEC))) / Double(NSEC_PER_SEC)
            DispatchQueue.main.asyncAfter(deadline: delayTime) {
                if (displayingHUD) {
                    progressHUD.show(in: keyWindow, animated: true)
                }
                displayingHUD = false
            }
        }
        
        progressHUD.progress = Float(progress)
    }
 }
 
 func displayIndeterminateProgress(_ message:String? = nil) {
    DispatchQueue.main.async {
        guard let keyWindow = UIApplication.shared.keyWindow else {
            return
        }
        
        if !displayingHUD && !(progressHUD.isVisible) {
            displayingHUD = true
            progressHUD.textLabel.text = message
            progressHUD.indicatorView = JGProgressHUDIndeterminateIndicatorView()
            progressHUD.animation = JGProgressHUDFadeZoomAnimation()
            
            let delayTime = DispatchTime.now() + Double(Int64(0.5 * Double(NSEC_PER_SEC))) / Double(NSEC_PER_SEC)
            DispatchQueue.main.asyncAfter(deadline: delayTime) {
                if (displayingHUD) {
                    progressHUD.show(in: keyWindow, animated: true)
                }
                displayingHUD = false
            }
        }
    }
 }
 
 func displayMessage(_ message: String) {
    DispatchQueue.main.async {
        guard let keyWindow = UIApplication.shared.keyWindow else {
            return
        }
        let indicator = JGProgressHUD(style: JGProgressHUDStyle.dark)
        indicator.textLabel.text = message
        indicator.indicatorView = JGProgressHUDIndicatorView()
        indicator.show(in: keyWindow)
        indicator.dismiss(afterDelay: 2.0)
    }
 }
 
 func hideProgress() {
    displayingHUD = false
    DispatchQueue.main.async {
        progressHUD.dismiss(afterDelay: 0.5)
    }
 }
 
 let errorHUD = JGProgressHUD(style: JGProgressHUDStyle.dark)
 
 func displayError(message: String) {
    DispatchQueue.main.async {
        guard let keyWindow = UIApplication.shared.keyWindow else {
            return
        }
        errorHUD.textLabel.text = message
        errorHUD.indicatorView = JGProgressHUDErrorIndicatorView()
        errorHUD.show(in: keyWindow)
        errorHUD.dismiss(afterDelay: 3.0)
    }
 }
 
 func applyPlainShadow(_ view: UIView, offset:CGSize, radius:CGFloat=0, opacity:Float=0.3) {
    let layer = view.layer
    
    layer.shadowColor = UIColor.black.cgColor
    layer.shadowOffset = offset
    layer.shadowOpacity = opacity
    layer.shadowRadius = radius > 0 ? radius : max(fabs(offset.width), fabs(offset.height)/2.0)
 }
 
 
 let CAMBRIAN_COLOR = UIColor(red: 22/255.0, green: 167/255.0, blue: 231/255.0, alpha: 1.0)
 
 func presentActionSheet(_ actionSheet:UIAlertController, viewController:UIViewController, button: UIBarButtonItem) {
    if let popoverController = actionSheet.popoverPresentationController {
        popoverController.barButtonItem = button
    }
    viewController.present(actionSheet, animated: true, completion: nil)
 }
 
 func presentActionSheet(_ actionSheet:UIAlertController, viewController:UIViewController, view: UIView) {
    if let popoverController = actionSheet.popoverPresentationController {
        popoverController.sourceRect = view.frame
        popoverController.sourceView = view
    }
    viewController.present(actionSheet, animated: true, completion: nil)
 }
 
 func confirmAction(_ presentingViewController:UIViewController, text:String, completion: @escaping (() -> Void)) {
    let alertController = UIAlertController(title: nil, message:
        text, preferredStyle: .alert)
    alertController.addAction(UIAlertAction(title: "Confirm", style: .default, handler: {
        (alert: UIAlertAction!) -> Void in
        completion()
    }));
    
    alertController.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: {
        (alert: UIAlertAction!) -> Void in
        
    }));
    
    presentingViewController.present(alertController, animated: true, completion: nil)
 }
 
 func share(_ presentingViewController:UIViewController, button: UIBarButtonItem, image: VisualizerImage, isImage: Bool, completion: (() -> Void)?=nil) {
    guard let imagePath = image.directoryPath?.path else {
        return
    }
    
    displayIndeterminateProgress()
    VisualizerProject.currentProject = image.project
    
    DispatchQueue.main.async(execute: {
        
        let baImage = CBRemodelingScene.getBeforeAfter(imagePath, isHorizontal: true)
        
        DispatchQueue.main.async(execute: {
            
            let renderer = ImageRenderer(image: image)
            guard let renderedImage = renderer.getRenderedImage(baImage) else {
                return
            }
            
            let avc = UIActivityViewController(activityItems: [renderedImage], applicationActivities:nil)
            
            avc.popoverPresentationController?.barButtonItem = button
            avc.completionWithItemsHandler = {
                (activity, success, items, error) in
                if let handler = completion , success {
                    handler()
                }
            }
            hideProgress()
            presentingViewController.present(avc, animated: true, completion: nil)
        })
    })
 }
 
 func uploadImage(url: URL,  _ imageID: String) {
    let storageRef = Storage.storage().reference()
    let imageRef = storageRef.child("images/\(imageID)")
    
    do {
        let zipFilePath = url.appendingPathExtension("zip")
        print(zipFilePath.path)
        try Zip.zipFiles(paths: [url], zipFilePath: zipFilePath, password: nil, progress: { (progress) in
            print("Progress \(progress)")
        })
        print("ZIP PATH : \(zipFilePath.path)")
        
        let uploadTask = imageRef.putFile(from: zipFilePath, metadata: nil) { (metadata, error) in
            if let error = error {
                // Uh-oh, an error occurred!
                print(error)
                return
            } else {
                // Metadata contains file metadata such as size, content-type, and download URL.
                print(zipFilePath.path)
            }
            
            do {
                try FileManager.default.removeItem(at: url.appendingPathExtension("zip"))        //delete zip to clean up
            } catch {
                print(error)
            }
            print("uploaded the zip")
        }
        
        
    } catch {
        print("something went wrong")
        return
    }
 }
 
 func uploadProject(url: URL,  _ projectId: String) {
    let storageRef = Storage.storage().reference()
    let projectRef = storageRef.child("projects/\(projectId)")
    do {
        let zipFilePath = url.appendingPathExtension("zip")
        print(zipFilePath.path)
        try Zip.zipFiles(paths: [url], zipFilePath: zipFilePath, password: nil, progress: { (progress) in
            print("Progress \(progress)")
        })
        print("ZIP PATH : \(zipFilePath.path)")
        
        let uploadTask = projectRef.putFile(from: zipFilePath, metadata: nil) { (metadata, error) in
            if let error = error {
                print(error)
                return
            } else {
                // Metadata contains file metadata such as size, content-type, and download URL.
                print(zipFilePath.path)
            }
            do {
                try FileManager.default.removeItem(at: url.appendingPathExtension("zip"))        //delete zip to clean up
            } catch {
                print(error)
            }
            print("uploaded the zip")
        }
    } catch {
        print("something went wrong")
        return
    }
 }
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
