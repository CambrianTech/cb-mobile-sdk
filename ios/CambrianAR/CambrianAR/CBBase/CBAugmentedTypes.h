//
//  CBAugmentedTypes.h
//  CambrianAR
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAugmentedTypes_h
#define CBAugmentedTypes_h

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, CBToolMode) {
    CBToolModeNone = 0,
    CBToolModeSelect,
    CBToolModeMove,
    CBToolModeFill,
    CBToolModeUnfill,
    CBToolModePaintbrush,
    CBToolModeEraser,
    CBToolModeFindColor,
};

typedef NS_ENUM(NSInteger, CBAssetType) {
    CBAssetTypeAll,
    CBAssetTypePaint,
    CBAssetTypeFloor,
    CBAssetTypeModel,
};

typedef NS_ENUM(NSInteger, CBLightingType) {
    CBLightingTypeNone,
    CBLightingTypeIncandescent,
    CBLightingTypeFluorescent,
    CBLightingTypeLEDWhite,
    CBLightingTypeLEDWarm,
    CBLightingTypeDaylightMorning,
    CBLightingTypeDaylight,
    CBLightingTypeDaylightEvening,
    CBLightingTypeDaylightOvercast,
};

#endif /* CBAugmentedTypes_h */
