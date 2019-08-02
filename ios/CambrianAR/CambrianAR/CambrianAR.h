//
//  CambrianAR Framework
//  CambrianAR
//
//  Created by Joel Teply on 8/1/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#import <UIKit/UIKit.h>

#define CB_PUBLIC __attribute__ ((visibility ("default")))
#define CB_LOCAL  __attribute__ ((visibility ("hidden")))

//! Project version number for HomeAugmentation.
FOUNDATION_EXPORT double HomeAugmentationVersionNumber;

//! Project version string for HomeAugmentation.
FOUNDATION_EXPORT const unsigned char HomeAugmentationVersionString[];

//#import <CambrianAR/CambrianAR.h>

//common
#import <CambrianAR/CBLicensing.h>

//AR other
#import <CambrianAR/CBTexture.h>
#import <CambrianAR/CBColoring.h>

//AR base objects
#import <CambrianAR/CBAugmentedTypes.h>
#import <CambrianAR/CBVideoCamera.h>
#import <CambrianAR/CBAugmentedView.h>
#import <CambrianAR/CBAugmentedAsset.h>
#import <CambrianAR/CBMaskedAsset.h>
#import <CambrianAR/CBAugmentedModel.h>
#import <CambrianAR/CBColorFinderView.h>

//Remodeling
#import <CambrianAR/CBRemodelingTypes.h>
#import <CambrianAR/CBRemodelingView.h>
#import <CambrianAR/CBRemodelingScene.h>
#import <CambrianAR/CBRemodelingPaint.h>
#import <CambrianAR/CBRemodelingFloor.h>
#import <CambrianAR/CBRemodelingFurniture.h>

