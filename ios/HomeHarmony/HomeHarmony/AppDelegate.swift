//
//  AppDelegate.swift
//  HomeDecoratorApp
//
//  Created by Joel Teply on 11/15/15.
//  Copyright © 2015 Joel Teply. All rights reserved.
//

import UIKit
import Alamofire
import SwiftyJSON
import Kingfisher
import Fabric
import Crashlytics
import Branch
import FirebaseCore
import AWSCore
import AWSCognito
import RealmSwift
import UserNotifications

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate, UNUserNotificationCenterDelegate, MessagingDelegate {

    var window: UIWindow?

    
    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplicationLaunchOptionsKey: Any]?) -> Bool {
        // Override point for customization after application launch.
        Fabric.with([Crashlytics.self])
        
//        let config = Realm.Configuration(deleteRealmIfMigrationNeeded: true)
//        Realm.Configuration.defaultConfiguration = config
        
        let cbAssetsDir = Bundle.main.resourceURL!.appendingPathComponent("CBAssets").path
        let cbConfig = CBRemodelingConfiguration(key: "f81b040d45bc43c688326e13b9877904", cbAssetPath: cbAssetsDir)
        cbConfig.primaryAssetPath = DataController.getWriteDirectory().appendingPathComponent("dbassets").path
        cbConfig.secondaryAssetPath = Bundle.main.resourceURL!.appendingPathComponent("dbassets").path
        CBLicensing.enable(cbConfig)
        
        FirebaseApp.configure()
        
        
        let branch: Branch = Branch.getInstance()
        branch.initSession(launchOptions: launchOptions, andRegisterDeepLinkHandler: {params, error in
            // If the key 'pictureId' is present in the deep link dictionary
            //print(params?.description)
            if error == nil && params?["+clicked_branch_link"] != nil && params?["Id"] != nil {
                //print("clicked link")
                if let window = self.window {
                    // add the project here and segue from the landing vc to the project
                    let controller = UIStoryboard(name: "Main", bundle: nil).instantiateViewController(withIdentifier: "LandingVC") as! LandingViewController
                    let nav = UINavigationController(rootViewController: controller)
                    
                    window.rootViewController = nav
                    let projReader: ProjectReaderController!
                    if (params?["isImage"] as! String == "true") {
                        projReader = ProjectReaderController(ID: params?["Id"] as! String, isImage: true)
                    } else {
                        projReader = ProjectReaderController(ID: params?["Id"] as! String, isImage: false)
                    }
                    
                    projReader.delegate = controller
                    projReader.createStorageReference()
                    projReader.downloadZip(completed: {
                        ImageManager.sharedInstance.askForCameraAccess(controller, handler: { (status) -> Void in
                            if status == .authorized {
                                //continue
                                controller.performSegue(withIdentifier: "showVisualizer", sender: self)
                                window.makeKeyAndVisible()
                            } else {
                                ImageManager.sharedInstance.noCameraAccessAlert(controller, showSettings: true)
                            }
                        })
                    })
                }
            }
        })
        
        // Persist tutorials when app loads back up
        let defaults = UserDefaults.standard
        let hasSeenInitialTutorial = ["hasSeenInitialTutorial" : false]
        let hasSeenRotationTutorial = ["hasSeenRotationTutorial" : false]
        let hasSeenDetailsTutorial = ["hasSeenDetailTutorial" : false]
        let hasSeenFloorTutorial = ["hasSeenFloorTutorial" : false]
        let hasSeenPaintTutorial = ["hasSeenPaintTutorial" : false]
        let hasSeenProductsTutorial = ["hasSeenProductsTutorial" : false]
        let hasSeenCaptureTutorial = ["hasSeenCaptureTutorial" : false]
        defaults.register(defaults: hasSeenInitialTutorial)
        defaults.register(defaults: hasSeenRotationTutorial)
        defaults.register(defaults: hasSeenDetailsTutorial)
        defaults.register(defaults: hasSeenFloorTutorial)
        defaults.register(defaults: hasSeenPaintTutorial)
        defaults.register(defaults: hasSeenProductsTutorial)
        defaults.register(defaults: hasSeenCaptureTutorial)
        
        // Push Notification
        Messaging.messaging().delegate = self
        
        if #available(iOS 10.0, *) {
            // For iOS 10 display notification (sent via APNS)
            UNUserNotificationCenter.current().delegate = self
            
            let authOptions: UNAuthorizationOptions = [.alert, .badge, .sound]
//            UNUserNotificationCenter.current().requestAuthorization(
//                options: authOptions,
//                completionHandler: {_, _ in })
        } else {
            let settings: UIUserNotificationSettings =
                UIUserNotificationSettings(types: [.alert, .badge, .sound], categories: nil)
            application.registerUserNotificationSettings(settings)
        }
        application.registerForRemoteNotifications()
        //print("-----firebase token: \(String(describing: Messaging.messaging().fcmToken)) ----")
        
        // Amazon AWS stuff
        let credentialProvider = AWSCognitoCredentialsProvider(regionType: .USEast1, identityPoolId: "us-east-1:1db8ca1e-3634-46fd-b7a0-f29a86ec8cd0")
        let configuration = AWSServiceConfiguration(region: .USEast1, credentialsProvider: credentialProvider)
        AWSServiceManager.default().defaultServiceConfiguration = configuration
        
        return true
    }
    
    func application(_ app: UIApplication, open url: URL, options: [UIApplicationOpenURLOptionsKey : Any] = [:]) -> Bool {
        
        // pass the url to the handle deep link call
        let urlPath: String = url.path
        //print(urlPath)
        
        Branch.getInstance().application(app,
                                         open: url,
                                         options:options
        )
        return true
    }
    
    func application(_ application: UIApplication, open url: URL, sourceApplication: String?, annotation: Any) -> Bool {
        
        let urlPath: String = url.path
        //print(urlPath)
        
        // pass the url to the handle deep link call
        // if handleDeepLink returns true, and you registered a callback in initSessionAndRegisterDeepLinkHandler, the callback will be called with the data associated with the deep link
        if (!Branch.getInstance().handleDeepLink(url)) {
            // do other deep link routing for the Facebook SDK, Pinterest SDK, etc
        }
        return true
        
    }
    
    func application(_ application: UIApplication, didRegisterForRemoteNotificationsWithDeviceToken deviceToken: Data) {
        // Retrieve and use APN token
        Messaging.messaging().apnsToken = deviceToken
    }
    
    func messaging(_ messaging: Messaging, didReceiveRegistrationToken fcmToken: String) {
        //print("Firebase registration token: \(fcmToken)")
    }
    
    // Respond to Universal Links
    func application(_ application: UIApplication, continue userActivity: NSUserActivity, restorationHandler: @escaping ([Any]?) -> Void) -> Bool {
        // pass the url to the handle deep link call
        return Branch.getInstance().continue(userActivity);
    }

    func applicationWillResignActive(_ application: UIApplication) {
        // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    }
    
    func applicationWillTerminate(_ application: UIApplication) {
        // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
        // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
        DataController.sharedInstance.refreshSyncedDatabases()
    }
    
    func applicationDidEnterBackground(_ application: UIApplication) {
        AppShortcuts.sync()
    }

    func applicationDidBecomeActive(_ application: UIApplication) {
        // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
        
        AppShortcuts.sync()
    }

    @available(iOS 9.0, *)
    func application(_ application: UIApplication, performActionFor shortcutItem: UIApplicationShortcutItem, completionHandler: @escaping (Bool) -> Void) {
        
        if let window = self.window {
            AppShortcuts.performShortcut(window, shortcut: shortcutItem)
        }
    }

}

