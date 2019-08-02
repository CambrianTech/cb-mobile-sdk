//
//  SwatchViewController.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit

class SwatchViewController: UIViewController, UIWebViewDelegate {

    @IBOutlet weak var webview: UIWebView!
    
    var selectedColor: ProductColor?
    
    override func viewDidLoad() {
        super.viewDidLoad()

        if let path = Bundle.main.path(forResource: "html/swatchview", ofType: "html") {
            webview.loadRequest(URLRequest(url:URL(fileURLWithPath:path)))
        }
    }
    
    func webViewDidFinishLoad(_ webView: UIWebView) {
        
        if let variation = self.selectedColor?.variations.first,
            let product = self.selectedColor?.parents.first {
            
//            if (FileManager.default.fileExists(atPath: variation.diffusePath)) {
//                webview.stringByEvaluatingJavaScript(
//                    from: "loadTextureAtPath('\(variation.diffusePath)','\(variation.normalPath)','\(variation.roughnessPath)', \(product.width), \(product.height))")
//            } else {
//                webview.stringByEvaluatingJavaScript(
//                    from: "alert('\(variation.diffusePath) is an invalid path!')")
//            }
        }
        
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func closePressed(_ sender: Any) {
        self.dismiss(animated: true) {
            
        }
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destinationViewController.
        // Pass the selected object to the new view controller.
    }
    */

}
