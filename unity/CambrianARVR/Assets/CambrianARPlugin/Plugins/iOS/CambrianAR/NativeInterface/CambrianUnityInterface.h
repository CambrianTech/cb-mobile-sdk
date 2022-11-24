//
//  CambrianUnity.h
//  CambrianUnity
//
//  Created by Joel Teply on 5/18/18.
//

#import <CambrianUnity/CambrianUnity.h>
#import "CambrianARNative.h"
#import "UnityAppController.h"

@interface CambrianUnityInterface : UnityAppController

@property (strong, nonatomic) CambrianARSession *session;

+ (CambrianUnityInterface *)sharedInstance;

- (void)startUnityInFrame:(CGRect)frame withScene:(NSString *)scene;

- (void)stopUnity;

- (void)refreshUnityView;

- (UIView *)view;

@end
