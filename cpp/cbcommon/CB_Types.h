//
//  CBImageTypes.h
//  Cambrian
//
//  Created by Joel Teply on 12/1/11.
//  Copyright (c) 2011 Cambrian Tech LLC. All rights reserved.
//

#ifndef CB_Types_h
#define CB_Types_h

#define _STRINGIZE(...) #__VA_ARGS__
#define STRINGIZE(...) _STRINGIZE(__VA_ARGS__)

#if defined(_WIN32) || defined(_WIN64)
#  include <gl/glew.h>
#  include <GL/gl.h>
#  include <GL/glu.h>
#elif __APPLE__
#  include "TargetConditionals.h"
#  if (TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR) || TARGET_OS_IPHONE
#    include <OpenGLES/ES3/gl.h>
#    include <OpenGLES/ES2/glext.h>
#  else
#    include <OpenGL/gl3.h>
#    include <OpenGL/gl3ext.h>
#  endif
#elif defined(__ANDROID__) || defined(ANDROID)
#  include <GLES3/gl3.h>
#  include <GLES2/gl2ext.h>
#elif defined(__linux__) || defined(__unix__) || defined(__posix__)
//#  include <GL/gl.h>
//#  include <GL/glu.h>
//#  include <GL/glext.h>
#else
#  error platform not supported.
#endif

#include <string>

using namespace std;

typedef enum {
    
    ColorClass_Whites = 0,
    ColorClass_Blacks = 1,
    ColorClass_Greys = 2,
    
    ColorClass_Reds = 3,
    ColorClass_Oranges = 4,
    ColorClass_Browns = 5,
    ColorClass_Yellows = 6,
    ColorClass_Greens = 7,
    ColorClass_Blues = 8,
    ColorClass_Purples = 9,
} ColorClass;

typedef enum
{
    CBColorTypeAll,
    CBColorTypeShadows,
    CBColorTypeMidtones,
    CBColorTypeHighlights,
}  CBColorType;

typedef enum
{
    ToolModeNone = 0,
    ToolModeSelect,
    ToolModeMove,
    ToolModeFill,
    ToolModeUnfill,
    ToolModePaintbrush,
    ToolModeEraser,
    ToolModeFindColor,
}  ToolMode;

typedef enum
{
    TouchStepBegan = 0, //ACTION_DOWN=0
    TouchStepEnded, //=ACTION_UP=1
    TouchStepMoved, //=ACTION_MOVE=2
    TouchStepCancelled, //=ACTION_CANCEL=3
    TouchStepTapped,
}  TouchStep;

typedef enum
{
    LightingTypeNone,
    LightingTypeIncandescent,
    LightingTypeFluorescent,
    LightingTypeLEDWhite,
    LightingTypeLEDWarm,
    LightingTypeSunrise,
    LightingTypeDaylightMorning,
    LightingTypeDaylight,
    LightingTypeDaylightEvening,
    LightingTypeDaylightOvercast,
}  LightingType;

typedef enum
{
    SheenFlat=-1,
    SheenMatte=0,
    SheenEggshell=1,
    SheenSatin=2,
    SheenSemiGloss=3,
    SheenGloss=4,
    SheenHighGloss=5,
}  Sheen;

typedef enum
{
    TransparencyNone,
    TransparencyClear,
    TransparencyTranslucent,
    TransparencyTransparent,
    TransparencySemiTransparent,
    TransparencySolid,
    TransparencyUltraSolid,
}  Transparency;

typedef enum
{
    FinishNone,
    FinishAgedPatina,
    FinishAntiqueLeather,
    FinishBrightCanvas,
    FinishMatte,
    FinishChalkboard,
    FinishWhiteboard,
    FinishCrackle,
    FinishFrosted,
    FinishGlitter,
    FinishGlowInTheDark,
    FinishSand,
    FinishHammered,
    FinishDenim,
    FinishMetallic,
    FinishRiverRock,
    FinishSlipResistant,
    FinishSuede,
    FinishWetLook,
}  Finish;

typedef enum {
    PaintPointType_All = 0,
    PaintPointType_Paint,
    PaintPointType_Surface,
    PaintPointType_Model,
} PaintPointType;

#endif
