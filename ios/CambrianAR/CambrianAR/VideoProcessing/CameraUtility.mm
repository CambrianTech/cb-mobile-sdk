//
//  CameraUtility.m
//  VisionBall
//
//  Created by Joel Teply on 4/30/11.
//  Copyright 2011 Digital Rising LLC. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>
#import <UIKit/UIKit.h>

#import "CameraUtility.h"

@implementation CameraUtility

+ (id) initCapture:(AVCaptureDevice *)videoCaptureDevice 
          delegate:(id<AVCaptureVideoDataOutputSampleBufferDelegate>)delegate
            preset:(NSString *) preset;
{
    if (!videoCaptureDevice) return nil;
    
	//set up device
	[videoCaptureDevice lockForConfiguration:nil];
	if ([videoCaptureDevice hasTorch]) {
		videoCaptureDevice.torchMode = AVCaptureTorchModeAuto;
	}
	if ([videoCaptureDevice isFocusModeSupported:AVCaptureFocusModeContinuousAutoFocus]) {
		videoCaptureDevice.focusMode = AVCaptureFocusModeContinuousAutoFocus;
	} 
    if ([videoCaptureDevice isExposureModeSupported:AVCaptureExposureModeContinuousAutoExposure]) {
        videoCaptureDevice.exposureMode = AVCaptureExposureModeContinuousAutoExposure;
    }
	
	[videoCaptureDevice unlockForConfiguration];
	
	/*We setup the input*/
	AVCaptureDeviceInput *captureInput = [AVCaptureDeviceInput 
										  deviceInputWithDevice:videoCaptureDevice 
										  error:nil];
	
	/*We setupt the output*/
	AVCaptureVideoDataOutput *captureOutput = [[AVCaptureVideoDataOutput alloc] init];
    
    //[videoConnection setVideoOrientation:AVCaptureVideoOrientationLPortrait];

	captureOutput.alwaysDiscardsLateVideoFrames = YES; 
	 
	/*We create a serial queue to handle the processing of our frames*/
	dispatch_queue_t queue;
	queue = dispatch_queue_create("cameraQueue", NULL);
	[captureOutput setSampleBufferDelegate:delegate queue:queue];
	//dispatch_release(queue);
	// Set the video output to store frame in BGRA (It is supposed to be faster)
    //outputSettings = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:kCVPixelFormatType_420YpCbCr8BiPlanarFullRange], kCVPixelBufferPixelFormatTypeKey, nil];
    NSDictionary* videoSettings = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], kCVPixelBufferPixelFormatTypeKey, nil];

	[captureOutput setVideoSettings:videoSettings];
	
	/*And we create a capture session*/
	AVCaptureSession *captureSession = [[AVCaptureSession alloc] init];
	/*We add input and output*/
    captureSession.sessionPreset = preset;
	[captureSession addInput:captureInput];
	[captureSession addOutput:captureOutput];
   ;
	
    return captureSession;
}

+ (CVPixelBufferRef) rotateBuffer: (CMSampleBufferRef) sampleBuffer
{
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(imageBuffer,0);
    
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);
    
    void *src_buff = CVPixelBufferGetBaseAddress(imageBuffer);
    
    NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey,
                             [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey,
                             nil];
    
    CVPixelBufferRef pxbuffer = NULL;
    //CVReturn status = CVPixelBufferPoolCreatePixelBuffer (NULL, _pixelWriter.pixelBufferPool, &pxbuffer);
    CVReturn status = CVPixelBufferCreate(kCFAllocatorDefault, width,
                                          height, kCVPixelFormatType_32BGRA, (__bridge CFDictionaryRef) options, 
                                          &pxbuffer);
    
    assert(status == kCVReturnSuccess && pxbuffer != NULL);
    
    CVPixelBufferLockBaseAddress(pxbuffer, 0);
    void *dest_buff = CVPixelBufferGetBaseAddress(pxbuffer);
    assert(dest_buff != NULL);
    
    int *src = (int*) src_buff ;
    int *dest= (int*) dest_buff ;
    size_t count = (bytesPerRow * height) / 4 ;
    while (count--) {
        *dest++ = *src++;
    }
    
    //Test straight copy.
    //memcpy(pxdata, baseAddress, width * height * 4) ;
    CVPixelBufferUnlockBaseAddress(pxbuffer, 0);
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    return pxbuffer;
}

+ (AVCaptureDevice *) getVideoCameraWithPosition:(AVCaptureDevicePosition)position;
{
    //  look at all the video devices and get the first one that's on the front
    NSArray *videoDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in videoDevices)
    {
        if (device.position == position)
        {
            return device;
        }
    }
    
    return NULL;
}

+ (UIImage *) getImageFromSampleBuffer:(CMSampleBufferRef)sampleBuffer scale:(CGFloat)scale;
{	
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer); 
    /*Lock the image buffer*/
    CVPixelBufferLockBaseAddress(imageBuffer,0); 
    /*Get information about the image*/
    uint8_t *baseAddress = (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer); 
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer); 
    size_t width = CVPixelBufferGetWidth(imageBuffer); 
    size_t height = CVPixelBufferGetHeight(imageBuffer);  
    
    /*Create a CGImageRef from the CVImageBufferRef*/
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB(); 
    
    
    CGContextRef newContext = CGBitmapContextCreate(baseAddress, width, height, 8, bytesPerRow, colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
	
	
    CGImageRef newImage = CGBitmapContextCreateImage(newContext); 
    
    CGContextRef resized = CGBitmapContextCreate(NULL, width * scale, height * scale, 8, bytesPerRow, colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
	CGContextDrawImage(resized, CGRectMake(0, 0, width * scale, height * scale), newImage);
	CGImageRef ref = CGBitmapContextCreateImage(resized);
	//UIImage *result = [UIImage imageWithCGImage:ref];
	CGContextRelease(resized);
    /*We release some components*/
    CGContextRelease(newContext); 
    CGColorSpaceRelease(colorSpace);
    
    
	/*We display the result on the image view (We need to change the orientation of the image so that the video is displayed correctly).
	 Same thing as for the CALayer we are not in the main thread so ...*/
	UIImage *image= [UIImage imageWithCGImage:ref scale:1.0 orientation:UIImageOrientationRight];

	CGImageRelease(ref);
	/*We relase the CGImageRef*/
	CGImageRelease(newImage);
	
	/*We unlock the  image buffer*/
	CVPixelBufferUnlockBaseAddress(imageBuffer,0);
    
    return image;
}

@end
