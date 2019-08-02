//
//  CBAugmentedAsset.h
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "CBAugmentedTypes.h"

@class CBAugmentedScene;

NS_ASSUME_NONNULL_BEGIN

CB_PUBLIC
@interface CBAugmentedAsset : NSObject

@property (readonly, nonatomic) CBAugmentedScene* scene;
@property (readonly, nonatomic) CBAssetType assetType;
@property (strong, nonatomic, nonnull, readonly) NSString *assetID;
@property (strong, nonatomic) UIImage *preview;
@property (readonly, nonatomic) UIImage *thumbnail;
@property (readonly, nonatomic) BOOL isUtilized;
@property (assign, nonatomic) BOOL isSelected;

- (nullable NSString *) getUserData:(NSString *)key;

- (void) setUserData:(NSString *)key value:(NSString *)value;

- (nullable CBAugmentedAsset *) initWithAssetID:(NSString *)assetID;

+ (BOOL)canCreate;

@end

NS_ASSUME_NONNULL_END
