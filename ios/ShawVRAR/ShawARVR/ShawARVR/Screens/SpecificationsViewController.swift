//
//  SpecificationsViewController.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/21/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit
//import SDWebImage

extension String {
    
    func fileName() -> String {
        return NSURL(fileURLWithPath: self).deletingPathExtension?.lastPathComponent ?? ""
    }
    
    func fileExtension() -> String {
        return NSURL(fileURLWithPath: self).pathExtension ?? ""
    }
}


class SpecificationsViewController: UIViewController, UIWebViewDelegate {
    
    @IBOutlet weak var webview: UIWebView!
    
    var selectedProduct: Product?
    var isTemplate = false

    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    override func viewWillAppear(_ animated: Bool) {
        if let path = Bundle.main.path(forResource: "html/specifications", ofType: "html"),
            let product = self.selectedProduct {
            
            //webview.loadRequest(URLRequest(url:product.specsPath))
        }
    }

    func webViewDidFinishLoad(_ webView: UIWebView) {
//        <script type="text/javascript">
//        function loadHTMLContent(url) {
//            $.get(url, function(data){
//                $(".completespec-wrapper").append(data);
//            });
//        }
//        </script>
        if (isTemplate) {
            //let specsPath = self.selectedProduct?.specsPath ?? "no product set"
            //webview.stringByEvaluatingJavaScript(from: "loadHTMLContent('\(specsPath)')")
        }
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
