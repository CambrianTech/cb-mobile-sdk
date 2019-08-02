//
//  CBTexture.m
//  HomeAugmentation
//
//  Created by Joel Teply on 11/7/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#import "CBTexture.h"

#include <cbar/pipeline/CBP_Types.hpp>

cv::Vec3f toVec3f(GLKVector3 glkVector) {
    return cv::Vec3f(glkVector.x, glkVector.y, glkVector.z);
}

GLKVector3 toGLK3(cv::Vec3f cvVec3f) {
    GLKVector3 glk;
    glk.x = cvVec3f[0];
    glk.y = cvVec3f[1];
    glk.z = cvVec3f[2];
    return glk;
}

@implementation CBMaterial

- (id) init {
    self = [super init];
    
    cbpipe::CBP_Material defaultMaterial;
    
    self.AmbientColor = toGLK3(defaultMaterial.AmbientColor);
    self.DiffuseColor = toGLK3(defaultMaterial.DiffuseColor);
    self.SpecularColor = toGLK3(defaultMaterial.SpecularColor);
    
    self.Shininess = defaultMaterial.Shininess;
    
    return self;
}

@end

@implementation CBTexture

- (id) init {
    self = [super init];
    
    self.material = [CBMaterial new];
    self.scale = 1.0;
    
    
    return self;
}

- (cbpipe::CBP_Texture)getCBPTexture {
    cbpipe::CBP_Texture texture;
    texture.texturePath = [self.path UTF8String];
    texture.scale = self.scale;
    
    texture.material.AmbientColor = toVec3f(self.material.AmbientColor);
    texture.material.DiffuseColor = toVec3f(self.material.DiffuseColor);
    texture.material.SpecularColor = toVec3f(self.material.SpecularColor);
    texture.material.Shininess = self.material.Shininess;
    
    
    return texture;
}

@end
