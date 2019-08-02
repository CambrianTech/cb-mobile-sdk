//
//  CBTexture.h
//  HomeAugmentation
//
//  Created by Joel Teply on 11/7/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>

__attribute__((visibility("default")))
@interface CBMaterial : NSObject
    @property (nonatomic, assign) GLKVector3 AmbientColor;
    @property (nonatomic, assign) GLKVector3 DiffuseColor;
    @property (nonatomic, assign) GLKVector3 SpecularColor;
    @property (nonatomic, assign) float Shininess;

@end

__attribute__((visibility("default")))
@interface CBTexture : NSObject
    @property (nonatomic, strong) NSString* path;
    @property (nonatomic, assign) float scale;
    @property (nonatomic, strong) CBMaterial *material;
@end
