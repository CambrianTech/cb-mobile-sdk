//
//  CBLicensing.m
//  Cambrian
//
//  Created by Joel Teply on 11/3/13.
//
//

#import "CBLicensing.h"
#include <cbcommon/CB_Licensing.h>
#include <cbar/CBAR_Common.hpp>

NS_ASSUME_NONNULL_BEGIN

static CBConfiguration *sharedConfig;

@interface CBConfiguration(internal)

- (void)initialize;

@end

@implementation CBConfiguration

- (instancetype)initWithAPIKey:(NSString *)apiKey cbAssetPath:(NSString *)cbAssetPath {
    if (self = [super init]) {
        NSString *bundleID  = [[NSBundle mainBundle] bundleIdentifier];
        cb::CB_Licensing::enableWithKey([apiKey UTF8String], [bundleID UTF8String]);
        
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDirectory = [paths objectAtIndex:0]; // Get documents folder
        
        self.cbAssetPath = cbAssetPath;
        self.loggingPath = [documentsDirectory stringByAppendingPathComponent:@"/image_logging"];
        self.workingPath = [documentsDirectory stringByAppendingPathComponent:@"/cbar"];
    }
    return self;
}

@end


@implementation CBRemodelingConfiguration

- (instancetype)initWithKey:(NSString *)apiKey cbAssetPath:(NSString *)cbAssetPath; {
    if (self = [super initWithAPIKey:apiKey cbAssetPath:cbAssetPath]) {
        
    }
    return self;
}

- (void)initialize {
    
    cbar::CBConfig cbConfig;
    
    if (self.cbAssetPath) {
        cbConfig.cbAssetPath = [self.cbAssetPath UTF8String];
    }
    if (self.workingPath) {
        cbConfig.workingPath = [self.workingPath UTF8String];
    }
    if (self.loggingPath) {
         cbConfig.loggingPath = [self.loggingPath UTF8String];
    }
    if (self.primaryAssetPath) {
        cbConfig.primaryAssetPath = [self.primaryAssetPath UTF8String];
        cbConfig.secondaryAssetPath = cbConfig.primaryAssetPath;
    }
    if (self.secondaryAssetPath) {
        cbConfig.secondaryAssetPath = [self.secondaryAssetPath UTF8String];
    }
    
    cbConfig.mode = cbar::CBOperationModeUnity;
    
    cbar::CB_Initialize(cbConfig);
}

@end

@implementation CBLicensing

+ (BOOL)isEnabled {
    return cb::CB_Licensing::isEnabled();
}

+ (BOOL)enable:(CBConfiguration *)config {
    [config initialize];

    if ([self isEnabled]) {
        sharedConfig = config;
        return YES;
    }

    return NO;
}

@end

NS_ASSUME_NONNULL_END
