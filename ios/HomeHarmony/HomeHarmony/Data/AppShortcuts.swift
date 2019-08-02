//
//  Shortcuts.swift
//  Prestige
//
//  Created by Joel Teply on 7/20/16.
//  Copyright © 2016 Cambrian. All rights reserved.
//

import Foundation

class AppShortcut : UIMutableApplicationShortcutItem {
    var segue:String
    
    init(type:String, title:String, icon:String, segue:String) {
        self.segue = segue
        let translatedTitle = NSLocalizedString(title, comment:title)
        let iconImage = UIApplicationShortcutIcon(templateImageName: icon)
        super.init(type: type, localizedTitle:translatedTitle, localizedSubtitle:nil, icon:iconImage, userInfo:nil)
    }
}

class AppShortcuts {
    
    static var shortcuts:[AppShortcut] = []
    
    class func sync() {
        
        var newShortcuts:[AppShortcut] = []
        
        //reverse order for display
        newShortcuts.append(AppShortcut(type: "explore-color", title: "Explore Color", icon:"ic_palette_black_24px", segue: "exploreColor"))
        
        newShortcuts.append(AppShortcut(type: "samples", title: "Sample Rooms", icon:"ic_photo_black_24px", segue: "showSamples"))
        
        //conditionally add an item like this:
        if (!VisualizerProject.currentProject.isEmpty) {
            newShortcuts.append(AppShortcut(type: "projects", title: "My Projects", icon:"ic_photo_black_24px", segue: "showProjects"))
        }
        
        newShortcuts.append(AppShortcut(type: "visualizer", title: "Paint Visualizer", icon:"ic_photo_camera_black_24px", segue: "showPainter"))
        
        UIApplication.shared.shortcutItems = newShortcuts
        shortcuts = newShortcuts
    }
    
    class func performShortcut(_ window:UIWindow, shortcut:UIApplicationShortcutItem) {

        sync()
        
        if let shortcutItem = shortcuts.filter({ $0.type == shortcut.type}).first {
            
            if let rootNavigationViewController = window.rootViewController as? UINavigationController,
                let landingViewController = rootNavigationViewController.viewControllers.first {
                //Pop to root view controller so that approperiete segue can be performed
                rootNavigationViewController.popToRootViewController(animated: false)
                
                landingViewController.performSegue(withIdentifier: shortcutItem.segue, sender: self)
            }
        }
    }
}
