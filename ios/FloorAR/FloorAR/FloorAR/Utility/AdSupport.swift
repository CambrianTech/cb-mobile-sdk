//
//  AdSupport.swift
//  DesignUp
//
//  Created by Joel Teply on 1/11/20.
//  Copyright © 2020 Joel Teply. All rights reserved.
//

import Foundation
import AdSupport
import FBAudienceNetwork

let fbAdBannerType:FBAdSize = UIDevice.current.userInterfaceIdiom == .pad ? kFBAdSizeHeight90Banner : kFBAdSizeHeight50Banner

let fbAdBannerPlacementID = UIDevice.current.userInterfaceIdiom == .pad ? "1088997268122110_1089840158037821" : "1088997268122110_1089840158037821"
var adsEnabled = true

func printIdentifierForAdvertising() {
    // Check whether advertising tracking is enabled
    guard ASIdentifierManager.shared().isAdvertisingTrackingEnabled else {
        print("This device does not support ads: isAdvertisingTrackingEnabled=false")
        return
    }

    // Get and return IDFA
    return print("This device's IDFA is: " + ASIdentifierManager.shared().advertisingIdentifier.uuidString)
}
