//
//  ImageManager.swift
//  HomeDecoratorApp
//
//  Created by Joel Teply on 12/10/15.
//  Copyright © 2015 Joel Teply. All rights reserved.
//

import Foundation
import Photos
import CoreGraphics

class ImageManager:NSObject, UIImagePickerControllerDelegate, UINavigationControllerDelegate {
    
    static var sharedInstance = ImageManager()
    
    var videoStatus = AVAuthorizationStatus.notDetermined
    var photoStatus = PHAuthorizationStatus.notDetermined
    
    override init() {
        super.init()
        self.videoStatus = AVCaptureDevice.authorizationStatus(for: AVMediaType.video)
        self.photoStatus = PHPhotoLibrary.authorizationStatus();
    }
    
    var pickerCallback:((_ image: UIImage?) -> Void)!
    
    func imagePickerController(_ picker: UIImagePickerController, didFinishPickingMediaWithInfo info: [String : Any]) {
        let chosenImage = info[UIImagePickerControllerOriginalImage] as! UIImage
        
        picker.dismiss(animated: true, completion: nil)
        
        if let cb = pickerCallback {
            cb(chosenImage)
        }
        self.pickerCallback = nil
    }
    
    func imagePickerControllerDidCancel(_ picker: UIImagePickerController) {
        picker.dismiss(animated: true, completion: nil)
        
        if let cb = pickerCallback {
            cb(nil)
        }
        self.pickerCallback = nil
    }
    
    func proceedWithCameraAccess(_ controller: UIViewController, handler: @escaping (() -> Void)) {
        
        if (self.videoStatus == .authorized) {
            DispatchQueue.main.async(execute: handler)
        } else {
            self.askForCameraAccess(controller, handler: { (status) -> Void in
                if status == .authorized {
                    //continue
                    DispatchQueue.main.async(execute: handler)
                } else {
                    self.noCameraAccessAlert(controller, showSettings: true)
                }
            })
        }
    }
    
    func askForCameraAccess(_ controller: UIViewController, handler: ((AVAuthorizationStatus) -> Void)!) {
        AVCaptureDevice.requestAccess(for: AVMediaType.video, completionHandler: {
            granted in
            
            self.videoStatus = AVCaptureDevice.authorizationStatus(for: AVMediaType.video)
            
            if let handler = handler {
                DispatchQueue.main.async(execute: {
                    handler(self.videoStatus)
                })
            }
        })
    }
    
    func proceedWithPhotoAccess(_ controller: UIViewController, handler: @escaping (() -> Void)) {
        if (self.photoStatus == .authorized) {
            DispatchQueue.main.async(execute: handler)
        } else {
            self.askForPhotosAccess(controller, handler: { (status) -> Void in
                if status == .authorized {
                    //continue
                    DispatchQueue.main.async(execute: handler)
                } else {
                    self.noCameraAccessAlert(controller, showSettings: true)
                }
            })
        }
    }
    
    func askForPhotosAccess(_ controller: UIViewController, handler: ((PHAuthorizationStatus) -> Void)!) {
        PHPhotoLibrary.requestAuthorization { (status) -> Void in
            self.photoStatus = status
            
            if let handler = handler {
                DispatchQueue.main.async(execute: {
                    handler(self.photoStatus)
                })
            }
        }
    }
    
    func hasCameraDevice() -> Bool {
        return UIImagePickerController.isCameraDeviceAvailable(UIImagePickerControllerCameraDevice.rear)
    }
    
    func hasCameraOption() -> Bool {
        self.videoStatus = AVCaptureDevice.authorizationStatus(for: AVMediaType.video)
        return hasCameraDevice() && (self.videoStatus == .authorized || self.videoStatus == .notDetermined)
    }
    
    func hasPhotoOption() -> Bool {
        self.photoStatus = PHPhotoLibrary.authorizationStatus();
        return self.photoStatus == .authorized || self.photoStatus == .notDetermined
    }
    
    func getImage(_ viewController:UIViewController, type:UIImagePickerControllerSourceType?=nil, callback:@escaping (_ image: UIImage?) -> Void) -> Bool {
        
        self.pickerCallback = callback
        
        self.videoStatus = AVCaptureDevice.authorizationStatus(for: AVMediaType.video)
        self.photoStatus = PHPhotoLibrary.authorizationStatus();
        
        if let type = type {
            if type == .camera {
                if (hasCameraDevice()) {
                    showCamera(viewController)
                    return true
                } else {
                    return false
                }
            } else {
                showPhotoLibrary(viewController, type:type)
                return true
            }
        }
        
        if (hasCameraOption() && hasPhotoOption()) {
            let optionMenu = UIAlertController(title: nil, message: "Choose Image Source", preferredStyle: .actionSheet)
            
            let cameraAction = UIAlertAction(title: "Device Camera", style: .default, handler: {
                (alert: UIAlertAction!) -> Void in
                self.showCamera(viewController)
            })
            let libraryAction = UIAlertAction(title: "Photo Library", style: .default, handler: {
                (alert: UIAlertAction!) -> Void in
                self.showPhotoLibrary(viewController, type:.photoLibrary)
            })
            
            //
            let cancelAction = UIAlertAction(title: "Cancel", style: .cancel, handler: {
                (alert: UIAlertAction!) -> Void in
                print("Cancelled")
                callback(nil)
                self.pickerCallback = nil
            })
            
            optionMenu.addAction(cameraAction)
            optionMenu.addAction(libraryAction)
            optionMenu.addAction(cancelAction)
            
            presentActionSheet(optionMenu, viewController: viewController, view: viewController.view)

            return true
        } else if (hasCameraOption()) {
            self.showCamera(viewController)
            return true
        } else if (hasPhotoOption()) {
            self.showPhotoLibrary(viewController, type:.photoLibrary)
            return true
        }
        
        return false
    }
    
    func showCamera(_ viewController:UIViewController) {
        
        let hasPermission:Bool = self.videoStatus == .authorized
        
        if (!hasPermission && self.videoStatus == .notDetermined) {
            self.proceedWithCameraAccess(viewController, handler: {
                self.showCamera(viewController)
            })
        } else if (hasPermission) {
            let picker = UIImagePickerController()
            picker.delegate = self
            picker.allowsEditing = false
            picker.sourceType = .camera
            DispatchQueue.main.asyncAfter(
                deadline: DispatchTime.now() + Double(Int64(0.1 * Double(NSEC_PER_SEC))) / Double(NSEC_PER_SEC), execute: {
                    viewController.present(picker, animated: true, completion: nil)
            })
        } else {
            noCameraAccessAlert(viewController, showSettings: true)
        }
    }
    
    func showPhotoLibrary(_ viewController:UIViewController, type:UIImagePickerControllerSourceType) {
        
        let hasPermission:Bool = self.photoStatus == .authorized
        
        if (!hasPermission && self.photoStatus == .notDetermined) {
            self.proceedWithPhotoAccess(viewController, handler: {
                self.showPhotoLibrary(viewController, type: type)
            })
        } else if (hasPermission) {
            let picker = UIImagePickerController()
            picker.delegate = self
            picker.allowsEditing = false
            picker.sourceType = type
            DispatchQueue.main.asyncAfter(
                deadline: DispatchTime.now() + Double(Int64(0.1 * Double(NSEC_PER_SEC))) / Double(NSEC_PER_SEC), execute: {
                    viewController.present(picker, animated: true, completion: nil)
            })
        } else {
            noPhotosAccessAlert(viewController, showSettings: true)
        }
    }
    
    func noCameraAccessAlert(_ controller: UIViewController, showSettings:Bool) {
        
        DispatchQueue.main.async { () -> Void in
            let title = NSLocalizedString("Camera Access", comment:"")
            let message = NSLocalizedString("Camera access has been disallowed. Please enable it in settings and then return to this app.", comment:"")
            
            
            let alert = UIAlertController(title: title, message: message, preferredStyle: UIAlertControllerStyle.alert)
            alert.addAction(UIAlertAction(title: NSLocalizedString("Cancel", comment:""), style: .cancel, handler: nil))
            
            if (showSettings) {
                alert.addAction(UIAlertAction(title: NSLocalizedString("Go to Settings", comment:""), style: .default, handler: { action in
                    UIApplication.shared.openURL(URL(string: UIApplicationOpenSettingsURLString)!)
                }))
            }
            
            DispatchQueue.main.asyncAfter(
                deadline: DispatchTime.now() + Double(Int64(0.1 * Double(NSEC_PER_SEC))) / Double(NSEC_PER_SEC), execute: {
                    controller.present(alert, animated: true, completion: nil)
            })
        }
    }
    
    func noPhotosAccessAlert(_ controller: UIViewController, showSettings:Bool) {
        
        DispatchQueue.main.async { () -> Void in
            let title = NSLocalizedString("Photo Gallery Access", comment:"")
            let message = NSLocalizedString("Photo Gallery access has been disallowed. Please enable it in settings and then return to this app.", comment:"")
            
            let alert = UIAlertController(title: title, message: message, preferredStyle: UIAlertControllerStyle.alert)
            alert.addAction(UIAlertAction(title: NSLocalizedString("Cancel", comment:""), style: .cancel, handler: nil))
            
            if (showSettings) {
                alert.addAction(UIAlertAction(title: NSLocalizedString("Go to Settings", comment:""), style: .default, handler: { action in
                    UIApplication.shared.openURL(URL(string: UIApplicationOpenSettingsURLString)!)
                }))
            }
            
            DispatchQueue.main.asyncAfter(
                deadline: DispatchTime.now() + Double(Int64(0.1 * Double(NSEC_PER_SEC))) / Double(NSEC_PER_SEC), execute: {
                    controller.present(alert, animated: true, completion: nil)
            })
        }
    }
    
    class func averageColor(_ image: UIImage) -> UIColor {
        let colorSpace = CGColorSpaceCreateDeviceRGB()

        let rgba = UnsafeMutablePointer<CUnsignedChar>.allocate(capacity: 4)
        let bitmapInfo:CGBitmapInfo = [.byteOrder32Big, CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedLast.rawValue)]
        
        let context = CGContext(data: rgba, width: 1, height: 1, bitsPerComponent: 8, bytesPerRow: 4, space: colorSpace, bitmapInfo: bitmapInfo.rawValue)
        context?.draw(image.cgImage!, in: CGRect(x: 0, y: 0, width: 1, height: 1))
        //CGColorSpaceRelease(colorSpace)
        //CGContextRelease(context)
        
        if rgba[3] > 0 {
            let alpha: CGFloat = (CGFloat(rgba[3])) / 255.0
            let multiplier: CGFloat = alpha / 255.0
            return UIColor(red: (CGFloat(rgba[0])) * multiplier, green: (CGFloat(rgba[1])) * multiplier, blue: (CGFloat(rgba[2])) * multiplier, alpha: alpha)
        }
        else {
            return UIColor(red: (CGFloat(rgba[0])) / 255.0, green: (CGFloat(rgba[1])) / 255.0, blue: (CGFloat(rgba[2])) / 255.0, alpha: (CGFloat(rgba[3])) / 255.0)
        }

    }
    
    class func imageNeedsDarkText(_ image:UIImage) -> Bool {
        
        let color = ImageManager.averageColor(image)
        
        var intensity:CGFloat = 0
        var saturation:CGFloat = 0
        color.getHue(nil, saturation: &saturation, brightness: &intensity, alpha: nil)
        
        return needsDarkText(0, saturation: saturation, brightness: intensity * 1.2)
    }
    
    class func needsDarkText(_ color:UIColor) -> Bool {
        var intensity:CGFloat = 0
        var saturation:CGFloat = 0
        color.getHue(nil, saturation: &saturation, brightness: &intensity, alpha: nil)
        
        return needsDarkText(0, saturation: saturation, brightness: intensity)
    }
    
    class func needsDarkText(_ hue:CGFloat, saturation:CGFloat, brightness:CGFloat) -> Bool {
        return brightness > 0.77 && saturation < 0.3
    }
    
    class func imageNeedsLightText(_ image:UIImage) -> Bool {
        
        let color = ImageManager.averageColor(image)
        
        var intensity:CGFloat = 0
        var saturation:CGFloat = 0
        color.getHue(nil, saturation: &saturation, brightness: &intensity, alpha: nil)
        
        return needsLightText(0, saturation: saturation, brightness: intensity * 1.2)
    }
    
    class func needsLightText(_ color:UIColor) -> Bool {
        var intensity:CGFloat = 0
        var saturation:CGFloat = 0
        color.getHue(nil, saturation: &saturation, brightness: &intensity, alpha: nil)
        
        return needsLightText(0, saturation: saturation, brightness: intensity)
    }
    
    class func needsLightText(_ hue:CGFloat, saturation:CGFloat, brightness:CGFloat) -> Bool {
        return brightness < 0.33
    }
}
