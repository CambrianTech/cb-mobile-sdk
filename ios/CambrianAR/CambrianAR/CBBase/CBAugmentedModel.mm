//
//  CBRemodelingPaint.m
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBAugmentedModel.h"

#import "CBARInternal.h"
#import "ImageTranslation.h"
#import "CBRemodelingTypes.h"
#include <cbar/CBAR_Common.hpp>
#import "CBARInternal.h"

#define SHOW_BOUNDING_CUBE 0

typedef NS_OPTIONS(NSUInteger, PositionModification) {
    PositionModificationNone     = 0,
    PositionModificationRotate   = 1 << 0,
    PositionModificationDrag     = 1 << 1,
};

@implementation SCNReferenceNode(bounding)
    - (SCNBox *)getBoundingCube {
        
        SCNVector3 boxMin, boxMax;
        [self getBoundingBoxMin:&boxMin max:&boxMax];
        
        
        SCNBox *cube = [SCNBox boxWithWidth:(boxMax.x - boxMin.x)
                                     height:(boxMax.y - boxMin.y)
                                     length:(boxMax.z - boxMin.z)
                              chamferRadius:0];
        
        cube.width = cube.width * 0.7;
        cube.length = cube.length * 0.7;
        
        SCNMaterial *noMaterial = [SCNMaterial new];
        
#if SHOW_BOUNDING_CUBE
        noMaterial.diffuse.contents = [UIColor colorWithRed:0.3 green:0.4 blue:0.5 alpha:1.0];
#else
        noMaterial.diffuse.contents = [UIColor colorWithWhite:0.0 alpha:0.0];
#endif
        
        cube.materials = @[noMaterial];
        
        return cube;
    }
@end

@interface CBAugmentedModel() {
    SCNMatrix4 m_initialTransform;
    SCNVector3 m_touchIntersection;
    
    bool m_hasLockedInTransform;
    PositionModification m_currentModifier;
    
    bool m_hasDropped;
    int64_t m_dropTime;
    int m_touchCount;
    SCNVector3 m_startTouchPosition;
}

@property (readonly, nonatomic) std::shared_ptr<cbscene::CBAR_Model> coreModel;
@property (strong, nonatomic) NSString *lastPath;
@property (strong, nonatomic) SCNBox *cube;

@end

@implementation CBAugmentedModel

@synthesize isEditingPosition = _isEditingPosition;

- (CBAssetType)assetType {
    return CBAssetTypeModel;
}

- (std::shared_ptr<cbscene::CBAR_Model>) coreModel {
    return std::dynamic_pointer_cast<cbscene::CBAR_Model>(self.coreAsset);
}

- (void) touchedGroundPlaneAt:(ARHitTestResult *)hitResult scene:(ARSCNView *)scene API_AVAILABLE(ios(11.0)) {
    
    if (!self.modelNode) {
        CBLog(@"Cannot drop model, no model loaded.");
        return;
    }
    
    m_touchIntersection = SCNVector3Make(
                                         hitResult.worldTransform.columns[3].x,
                                         hitResult.worldTransform.columns[3].y,
                                         hitResult.worldTransform.columns[3].z
                                         );
    
    if (m_hasDropped) {
        //check distance from point here.
        
        double distanceToDrop = Geometry::euclideanDistance(cv::Point2f(self.modelNode.position.x, self.modelNode.position.z),
                                                              cv::Point2f(m_touchIntersection.x, m_touchIntersection.z));
        
        if (distanceToDrop < 1.0) {
            CBLog(@"Already dropped, distance too close to drop again %f", distanceToDrop);
            return;
        }
    }
    
    m_hasDropped = YES;
    m_dropTime = sys_usec_time();
    
    float insertionYOffset = 2.0;
    SCNVector3 position = m_touchIntersection;
    position.y += insertionYOffset;
    
    self.modelNode.physicsBody.velocity = SCNVector3Zero;
    self.modelNode.physicsBody.angularDamping = 1.0;
    //self.modelNode.physicsBody.damping = 0.3;
    
    self.modelNode.position = position;
    
    [CBAugmentedView getInstance].arSceneVisible = true;
    
    if (self.modelNode.parentNode) {
        [self.modelNode removeFromParentNode];
    }
    
    CBLog(@"Dropping in model");
    m_hasLockedInTransform = false;
    [scene.scene.rootNode addChildNode:self.modelNode];
}

- (void)setPath:(NSString *) path scale:(float)scale {
    if (m_hasDropped) {
        return;
    }
    
    NSString *modelPath = [path stringByAppendingPathComponent:@"model.scn"];
    
    if ([modelPath isEqualToString:self.lastPath]) return;
    
    NSString *fullPath = [NSString stringWithUTF8String:getUserAssetPath([modelPath UTF8String]).c_str()];
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:fullPath]) {
        CBLog(@"No model found at path %@", fullPath);
        return;
    }
    
    NSURL *referenceURL = [NSURL fileURLWithPath:fullPath];
    SCNReferenceNode *referenceNode = [SCNReferenceNode referenceNodeWithURL:referenceURL];
    
    if (!referenceNode) return;
    
    [referenceNode load];
    
    //node.castsShadow = true;
    
    self.cube = [referenceNode getBoundingCube];
    
    SCNNode *containingNode = [SCNNode nodeWithGeometry:self.cube];
    [containingNode addChildNode:referenceNode];
    
    auto translate = SCNMatrix4MakeTranslation(0, -self.cube.height/2, 0);
    referenceNode.transform = SCNMatrix4Mult(translate, containingNode.transform);
    
    containingNode.geometry = self.cube;
    
    double volume = self.cube.width * self.cube.height * self.cube.length;
    
    
    // The physicsBody tells SceneKit this geometry should be manipulated by the physics engine
    containingNode.physicsBody = [SCNPhysicsBody bodyWithType:SCNPhysicsBodyTypeDynamic shape:nil];
    containingNode.physicsBody.mass = volume * 5;
    containingNode.physicsBody.categoryBitMask = CollisionCategoryCube;
    containingNode.physicsBody.restitution = 0.1;
    containingNode.physicsBody.friction = 0.8;
    
    self.lastPath = modelPath;
    self.modelNode = containingNode;
}

- (void) attachedToScene:(CBAugmentedScene *)scene {
    [super attachedToScene:scene];
}

- (void) detachedFromScene:(CBAugmentedScene *)scene {
    [self.modelNode removeFromParentNode];
    [super detachedFromScene:scene];
}

- (BOOL) isUtilized {
    return m_hasDropped;
}

- (void) setIsEditingPosition:(BOOL)value {
    if (value == _isEditingPosition) return;
    _isEditingPosition = value;
    if (value) {
        self.modelNode.physicsBody.type = SCNPhysicsBodyTypeKinematic;
        self.isSelected = true;
    } else {
        //turn on gravity again
        self.modelNode.position = self.modelNode.presentationNode.position;
        self.modelNode.physicsBody.type = SCNPhysicsBodyTypeDynamic;
        self.modelNode.physicsBody.velocity = SCNVector3Zero;
    }
    
    self.modelNode.opacity = value ? 0.7 : 1.0;
}

- (void) setIsSelected:(BOOL)value {
    [super setIsSelected:value];
    if (!value) {
        self.isEditingPosition = false;
    }
}

//thar be hackery aboot (corey as a pirate)
- (void) lockInTransform {
    
    //https://stackoverflow.com/questions/34092588/why-scnphysicsbody-resets-position-when-set-eulerangles/39799785#39799785
    
    if (!m_hasLockedInTransform) {
        self.modelNode.position = self.modelNode.presentationNode.position;
        //float distance = fabs(self.modelNode.position.y - m_touchIntersection.y);
        //CBLog(@"distance = %f", distance);
        m_hasLockedInTransform = YES;
    }
    
    m_initialTransform = self.modelNode.transform;
}

- (void) touchesCommon:(TouchStep&)step modification:(PositionModification)action {
    if (step == TouchStepBegan) {
        if (!self.isEditingPosition) {
            self.isEditingPosition = YES;
        }
        
        m_touchCount = 0;
        step = TouchStepBegan;
        m_currentModifier = action;
        [self lockInTransform];
    }
    else if (step != TouchStepMoved) {
        self.isEditingPosition = NO;
        m_currentModifier = PositionModificationNone;
    } else {
        m_touchCount ++;
    }
}

- (BOOL)isMyNode:(SCNNode *)node {
    if (node == self.modelNode) {
        return YES;
    } else if (node.parentNode) {
        return [self isMyNode:node.parentNode];
    }
    return NO;
}

- (void) touchedAtPoint:(CGPoint)touchPoint step:(TouchStep)step {
        
    if (!m_hasDropped || seconds_elapsed(m_dropTime) < 0.5) return;
    
    if (@available(iOS 11.0, *)) {
        
        if (step != TouchStepBegan && m_currentModifier != PositionModificationDrag) {
            return;
        }
        
        [self touchesCommon:step modification:PositionModificationDrag];
        
        if (m_currentModifier != PositionModificationDrag) return;
                
        SCNVector3 touchPosition = [CBAugmentedView.getInstance getGroundPosition3D:touchPoint];

        if (SCNVector3EqualToVector3(touchPosition, SCNVector3Zero)) return;
        
        
        if (step == TouchStepBegan) {
            m_startTouchPosition = SCNVector3Make(self.modelNode.position.x - touchPosition.x,
                                                  self.modelNode.position.y - touchPosition.y,
                                                  self.modelNode.position.z - touchPosition.z);
        }
        else if (step == TouchStepMoved && m_touchCount > 5) {
            
            SCNVector3 newPosition = SCNVector3Make(touchPosition.x + m_startTouchPosition.x,
                                                    self.modelNode.position.y,
                                                    touchPosition.z + m_startTouchPosition.z);
            
            double distanceToCenter = Geometry::euclideanDistance(cv::Point2f(self.modelNode.position.x, self.modelNode.position.z),
                                                                 cv::Point2f(newPosition.x, newPosition.z));
            
            double distanceToStart = Geometry::euclideanDistance(cv::Point2f(m_startTouchPosition.x, m_startTouchPosition.z),
                                                                  cv::Point2f(newPosition.x, newPosition.z));
            
            float maxDistance = 3.0;
            
            if (distanceToCenter < 0.01) return;
            
            float maxMagnitude = 0.07;//todo: calculate speed
            
            SCNVector3 offset = SCNVector3Make(newPosition.x - self.modelNode.position.x,
                                               0.0,
                                               newPosition.z - self.modelNode.position.z);
            float magnitude = fmin(maxMagnitude, sqrtf(offset.x * offset.x + offset.y * offset.y + offset.z * offset.z));
            
            if (distanceToStart > maxDistance) {
                //slow
                magnitude = 0.015;
            }
            
            offset.x *= magnitude;
            offset.y *= magnitude;
            offset.z *= magnitude;
            
            
            //[SCNTransaction begin];

            //[SCNTransaction setAnimationTimingFunction:[CAMediaTimingFunction functionWithName: kCAMediaTimingFunctionEaseInEaseOut]];
            
            //[SCNTransaction setAnimationDuration:CFTimeInterval(distanceToPlane * 300)];
            
            self.modelNode.position = SCNVector3Make(self.modelNode.position.x + offset.x,
                                                     newPosition.y,
                                                     self.modelNode.position.z + offset.z);
            
            //[SCNTransaction commit];
        }
    }
    
}

- (void) rotatedAtPoint:(CGPoint)point step:(TouchStep)step rotation:(float)amount {
    if (!m_hasDropped || seconds_elapsed(m_dropTime) < 0.5) return;
    
    [self touchesCommon:step modification:PositionModificationRotate];
    
    SCNMatrix4 rotation = SCNMatrix4MakeRotation(amount, 0, -0.1, 0);
    
    self.modelNode.transform = SCNMatrix4Mult(rotation, m_initialTransform);
}

+ (BOOL)canCreate {
    return false;
    //return hasARKit();
}

@end

