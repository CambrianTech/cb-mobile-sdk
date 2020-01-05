//
//  Networking.swift
//  FloorAR
//
//  Created by Joel Teply on 12/30/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

import Foundation
import Alamofire

#if DEBUG
    let flushCache = true
#else
    let flushCache = false
#endif


public struct Networking {
    static var isInternetAvailable:Bool
    {
        return NetworkReachabilityManager()!.isReachable
    }
}
