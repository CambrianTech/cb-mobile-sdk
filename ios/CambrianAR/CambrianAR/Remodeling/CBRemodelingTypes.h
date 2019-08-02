//
//  CBRemodelingTypes.h
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBRemodelingTypes_h
#define CBRemodelingTypes_h

typedef NS_ENUM(NSInteger, CBUndoChange) {
    CBUndoChangePaintColor,
    CBUndoChangePaintSheen,
    CBUndoChangeMask,
};

typedef NS_ENUM(NSInteger, CBTransparency) {
    CBTransparencyNone,
    CBTransparencyClear,
    CBTransparencyTranslucent,
    CBTransparencyTransparent,
    CBTransparencySemiTransparent,
    CBTransparencySolid,
    CBTransparencyUltraSolid,
};

typedef NS_ENUM(NSInteger, CBPaintSheen) {
    CBPaintSheenFlat,
    CBPaintSheenMatte,
    CBPaintSheenEggshell,
    CBPaintSheenSatin,
    CBPaintSheenSemiGloss,
    CBPaintSheenGloss,
    CBPaintSheenHighGloss,
};

typedef NS_ENUM(NSInteger, CBFinish) {
    CBFinishNone,
    CBFinishAgedPatina,
    CBFinishAntiqueLeather,
    CBFinishBrightCanvas,
    CBFinishMatte,
    CBFinishChalkboard,
    CBFinishWhiteboard,
    CBFinishCrackle,
    CBFinishFrosted,
    CBFinishGlitter,
    CBFinishGlowInTheDark,
    CBFinishSand,
    CBFinishHammered,
    CBFinishDenim,
    CBFinishMetallic,
    CBFinishRiverRock,
    CBFinishSlipResistant,
    CBFinishSuede,
    CBFinishWetLook,
};

#endif /* CBRemodelingTypes_h */

