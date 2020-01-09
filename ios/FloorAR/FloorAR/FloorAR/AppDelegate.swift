//
//  AppDelegate.swift
//  ShawARVR
//
//  Created by Joel Teply on 5/15/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    var window: UIWindow?

    internal func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        
        let documentDirectoryURL =  try! FileManager.default.url(for: .documentDirectory, in: .userDomainMask, appropriateFor: nil, create: true)
        
        let cbAssetsDir = Bundle.main.resourceURL!.appendingPathComponent("CBAssets").path
        let cbConfig = CBRemodelingConfiguration(key: "f81b040d45bc43c688326e13b9877904", cbAssetPath: cbAssetsDir)
        cbConfig.primaryAssetPath = Bundle.main.resourceURL!.appendingPathComponent("items").path
        cbConfig.secondaryAssetPath = Bundle.main.resourceURL!.appendingPathComponent("Assets").path
        
        cbConfig.workingPath = documentDirectoryURL.appendingPathComponent("cbar", isDirectory: true).path;
        
        CBLicensing.enable(cbConfig)
        
        NSSetUncaughtExceptionHandler { exception in
           print(exception)
           print(exception.callStackSymbols)
        }
                
        return true
    }

    func applicationWillResignActive(_ application: UIApplication) {
        // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    }

    func applicationDidEnterBackground(_ application: UIApplication) {
        // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
        // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
        // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
    }

    func applicationDidBecomeActive(_ application: UIApplication) {
        // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    }

    func applicationWillTerminate(_ application: UIApplication) {
        // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    }


}

