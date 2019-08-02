//
//  CambrianAR.h
//  CambrianAR
//
//  Created by Joel Teply on 12/4/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import <UIKit/UIKit.h>

#define CB_PUBLIC __attribute__ ((visibility ("default")))
#define CB_LOCAL  __attribute__ ((visibility ("hidden")))

//! Project version number for CambrianAR.
FOUNDATION_EXPORT double CambrianARVersionNumber;

//! Project version string for CambrianAR.
FOUNDATION_EXPORT const unsigned char CambrianARVersionString[];

//common
#import <CambrianSDK/CBLicensing.h>

//AR other
#import <CambrianSDK/CBTexture.h>
#import <CambrianSDK/CBColoring.h>

//AR base objects
#import <CambrianSDK/CBAugmentedTypes.h>
#import <CambrianSDK/CBAugmentedView.h>
#import <CambrianSDK/CBAugmentedAsset.h>
#import <CambrianSDK/CBSurfaceAsset.h>
#import <CambrianSDK/CBAugmentedModel.h>
#import <CambrianSDK/CBColorFinderView.h>

//Remodeling
#import <CambrianSDK/CBRemodelingTypes.h>
#import <CambrianSDK/CBRemodelingView.h>
#import <CambrianSDK/CBRemodelingScene.h>
#import <CambrianSDK/CBRemodelingPaint.h>
#import <CambrianSDK/CBRemodelingFloor.h>
