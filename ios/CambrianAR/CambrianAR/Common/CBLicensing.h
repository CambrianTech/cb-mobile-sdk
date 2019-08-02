//
//  CBLicensing.h
//  Cambrian
//
//  Created by Joel Teply on 11/3/13.
//
//

#import <Foundation/Foundation.h>
#import <UIKIt/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

__attribute__((visibility("default")))
@interface CBConfiguration : NSObject

@property (nonatomic, strong) NSString *cbAssetPath;
@property (nonatomic, strong) NSString *loggingPath;
@property (nonatomic, strong) NSString *workingPath;

/** Unavailable */
//- (instancetype)init NS_UNAVAILABLE;
//+ (instancetype)new NS_UNAVAILABLE;

@end

__attribute__((visibility("default")))
@interface CBLicensing : NSObject
+ (BOOL)enable:(CBConfiguration *)config;
+ (BOOL)isEnabled;

//- (instancetype)init NS_UNAVAILABLE;
//+ (instancetype)new NS_UNAVAILABLE;
@end

__attribute__((visibility("default")))
@interface CBRemodelingConfiguration : CBConfiguration

@property (nonatomic, strong) NSString *primaryAssetPath;
@property (nonatomic, strong) NSString *secondaryAssetPath;

- (instancetype)initWithKey:(NSString *)apiKey cbAssetPath:(NSString *)cbAssetPath;
@end


NS_ASSUME_NONNULL_END
