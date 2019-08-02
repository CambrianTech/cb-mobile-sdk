//
//  CBP_MatrixUtil.hpp
//  Cambrian
//
//  Created by Joel Teply on 10/7/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_MatrixUtil_hpp
#define CBP_MatrixUtil_hpp

#include <stdio.h>

#include <Eigen/Geometry>
#include <Eigen/QR>
#include <Eigen/LU>
#include <opencv2/opencv.hpp>

namespace cbpipe {
    
    class CBP_MatrixUtil {
        
    public:
        
        static Eigen::Matrix4f makeRotationMatrix(float pitchAroundX, float yawAroundY, float rollAroundZ);
        static Eigen::Matrix4f makeRotationMatrix(const cv::Vec4f &quaterionXYZW);
        
        static Eigen::Matrix4f makeProjectionMatrix(float fov, float aspect, float zNear, float zFar, bool leftHanded=false);
        static Eigen::Matrix4f makeProjectionMatrix(const Eigen::Matrix3f &cameraIntrinsics, float znear, float zfar);
        
        static Eigen::Quaternionf getQuaternion(const Eigen::Matrix3f &rotation);
        static Eigen::Quaternionf getAngularVelocity(const Eigen::Quaternionf &qStart, const Eigen::Quaternionf &qEnd, float deltaSeconds);
        static Eigen::Vector3f getTranslationalVelocity(const Eigen::Matrix4f &pStart, const Eigen::Matrix4f &pEnd, float deltaSeconds);
        
        static Eigen::Matrix4f makeScaleMatrix(float x,float y,float z);
        static Eigen::Matrix4f makeTranslationMatrix(float x, float y, float z);
        static Eigen::Matrix3f normalMatrix(const Eigen::Matrix4f& in);
        
        static Eigen::Matrix4f convertTo4x4Matrix(const Eigen::Matrix3f &matrix);
        static Eigen::Matrix3f convertTo3x3Matrix(const Eigen::Matrix4f &matrix);
        
        static Eigen::Vector3f getPosition(const Eigen::Matrix4f &transform);
        static Eigen::Vector3f getNormal(const Eigen::Matrix4f &transform);
        
        static cv::Mat_<float> getCameraMatrix(cv::Size frameSize, float HFOV);
        
        static Eigen::Matrix3f makeCameraMatrix(cv::Size frameSize, float HFOV);
        
        static Eigen::Vector3f unprojectPoint(const cv::Point2f& point2D, float winZ,
                                              const Eigen::Matrix4f& mvpInverse, const Eigen::Matrix4f& projection);
        
        static cv::Point3f projectPoint(const Eigen::Vector3f &position3D, const Eigen::Matrix4f& mvp);
        
        static cv::Mat rot2euler(const cv::Mat & rotationMatrix);
        static cv::Mat euler2rot(const cv::Mat & euler);
        
        static Eigen::Vector3f getEulerAngles(Eigen::Matrix3f &rotation);
        
        static Eigen::ParametrizedLine<float,3> projectPixelTo3DRay(const cv::Point2f& uv_rect, const cv::Size &frameSize, float HFOV);
        
        static bool getPlaneIntersection(const cv::Point2f &point2D,
                                  const cv::Size &viewSize,
                                  const cv::Size_<float> &fov,
                                  const Eigen::Hyperplane<float, 3> &hyperplane,
                                  const Eigen::Matrix4f &mvpInverted,
                                  float maxDistanceThreshold,
                                  Eigen::Vector3f &intersection);
        
        static Eigen::Vector3f getEulerAngles(const Eigen::Quaternionf &quat);
        
        //OpenGL, ARKit, SceneKit, are right-handed
        static Eigen::Vector3f toRightHanded(const Eigen::Vector3f &leftHanded);
        
        //Unity and DirectX are left-handed
        static Eigen::Vector3f toLeftHanded(const Eigen::Vector3f &rightHanded);
        
        static double angleBetweenVectors(const Eigen::Vector3f &vectorA, const Eigen::Vector3f &vectorB);
    };
}

#endif /* CBP_MatrixUtil_hpp */

