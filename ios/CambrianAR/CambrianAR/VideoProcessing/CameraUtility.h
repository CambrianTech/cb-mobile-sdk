//
//  CameraUtility.h
//  VisionBall
//
//  Created by Joel Teply on 4/30/11.
//  Copyright 2011 Digital Rising LLC. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>

@interface CameraUtility : NSObject {
    
}

+ (id) initCapture:(AVCaptureDevice *)videoCaptureDevice 
          delegate:(id<AVCaptureVideoDataOutputSampleBufferDelegate>)delegate
            preset:(NSString *) preset;

+ (AVCaptureDevice *) getVideoCameraWithPosition:(AVCaptureDevicePosition)position;

+ (UIImage *) getImageFromSampleBuffer:(CMSampleBufferRef)sampleBuffer scale:(CGFloat)scale;

+ (CVPixelBufferRef) rotateBuffer: (CMSampleBufferRef) sampleBuffer;

@end
