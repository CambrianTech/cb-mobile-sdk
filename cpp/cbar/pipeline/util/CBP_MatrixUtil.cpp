//
//  CBP_MatrixUtil.cpp
//  Cambrian
//
//  Created by Joel Teply on 10/7/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#include "CBP_MatrixUtil.hpp"
#include <opencv2/core/eigen.hpp>

namespace cbpipe {
    
    //https://stackoverflow.com/questions/42051965/how-to-convert-mesh-defined-in-a-right-handed-xyz-coordinate-system-to-a-left-ha
    //OpenGL, ARKit, SceneKit, are right-handed
    Eigen::Vector3f CBP_MatrixUtil::toRightHanded(const Eigen::Vector3f &leftHanded) {
        const float vals[] = {
            0,  0, -1,
            1,  0,  0,
            0,  1,  0,
        };
        
        return Eigen::Matrix3f(vals) * leftHanded;
    }
    
    //Unity and DirectX are left-handed
    Eigen::Vector3f CBP_MatrixUtil::toLeftHanded(const Eigen::Vector3f &rightHanded) {
        const float vals[] = {
            0,  1,  0,
            0,  0,  1,
            -1,  0,  0,
        };
        return Eigen::Matrix3f(vals) * rightHanded;
    }
    
    Eigen::Vector3f CBP_MatrixUtil::getPosition(const Eigen::Matrix4f &transform) {
        return transform.block(0, 3, 3, 1);
    }

    Eigen::Vector3f CBP_MatrixUtil::getNormal(const Eigen::Matrix4f &transform) {
        return transform.block(0, 1, 3, 1); //2nd column is normal
    }
    
    //https://answers.unity.com/questions/1461036/worldtoviewportpoint-and-viewporttoworldpointworld.html
    cv::Point3f CBP_MatrixUtil::projectPoint(const Eigen::Vector3f &point3D, const Eigen::Matrix4f& mvp) {
        Eigen::Vector4f point4 = Eigen::Vector4f(point3D.x(), point3D.y(), point3D.z(), 1.0f);
        Eigen::Vector4f result4 = mvp * point4;  // multiply 4 components
     
        result4.x() = result4.x() / (result4.w() * -2.0f) + 0.5f;
        result4.y() = result4.y() / (result4.w() * -2.0f) + 0.5f;
        
        return cv::Point3f(result4.x(), 1.0f - result4.y(), -result4.w());
    }
    
    //https://answers.unity.com/questions/1461036/worldtoviewportpoint-and-viewporttoworldpointworld.html
    Eigen::Vector3f CBP_MatrixUtil::unprojectPoint(const cv::Point2f& point2D, float winZ,
                                                   const Eigen::Matrix4f& mvpInverse, const Eigen::Matrix4f& projection) {
        
        Eigen::Vector4f projW = projection * Eigen::Vector4f(0, 0, winZ, 1);
        Eigen::Vector4f point4 = Eigen::Vector4f(1.0f - point2D.x * 2.0f,
                                                 2.0f * point2D.y - 1.0f,
                                                 projW.z() / projW.w(), 1);
        
        Eigen::Vector4f result4 = mvpInverse * point4;  // multiply 4 components
        
        result4 = result4 / result4.w();  // store 3 components of the resulting 4 components
        
        return Eigen::Vector3f(result4.x(), result4.y(), result4.z());
    }
    
    Eigen::Matrix4f CBP_MatrixUtil::makeRotationMatrix(float pitchAroundX, float yawAroundY, float rollAroundZ) {
        Eigen::AngleAxisf rollAngle(rollAroundZ, Eigen::Vector3f::UnitZ());
        Eigen::AngleAxisf yawAngle(yawAroundY, Eigen::Vector3f::UnitY());
        Eigen::AngleAxisf pitchAngle(pitchAroundX, Eigen::Vector3f::UnitX());
        
        Eigen::Quaternionf q = rollAngle * yawAngle * pitchAngle;
        return convertTo4x4Matrix(q.toRotationMatrix());
    }
    
    Eigen::Matrix4f CBP_MatrixUtil::makeRotationMatrix(const cv::Vec4f &quaterionXYZW) {
        Eigen::Quaternionf quat(quaterionXYZW[3], quaterionXYZW[0], quaterionXYZW[1], quaterionXYZW[2]);
        quat.normalize();
        return convertTo4x4Matrix(quat.toRotationMatrix());
    }
    
    Eigen::Quaternionf CBP_MatrixUtil::getQuaternion(const Eigen::Matrix3f &rotation) {
        Eigen::Quaternionf q(rotation);
        return q;
    }
    
    Eigen::Quaternionf CBP_MatrixUtil::getAngularVelocity(const Eigen::Quaternionf &qStart, const Eigen::Quaternionf &qEnd, float deltaSeconds) {
        const Eigen::Quaternionf distance = Eigen::Quaternionf(qEnd.w() - qStart.w(),
                                                               qEnd.x() - qStart.x(),
                                                               qEnd.y() - qStart.y(),
                                                               qEnd.z() - qStart.z());
        return Eigen::Quaternionf(distance.w() / deltaSeconds,
                                  distance.x() / deltaSeconds,
                                  distance.y() / deltaSeconds,
                                  distance.z() / deltaSeconds);
    }
    
    Eigen::Vector3f CBP_MatrixUtil::getTranslationalVelocity(const Eigen::Matrix4f &pStart, const Eigen::Matrix4f &pEnd, float deltaSeconds) {
        Eigen::Vector4f posA = pStart.col(3);
        Eigen::Vector4f posB = pEnd.col(3);
        Eigen::Vector4f distance = posB - posA;
        
        return Eigen::Vector3f(distance.x()/deltaSeconds,
                               distance.y()/deltaSeconds,
                               distance.z()/deltaSeconds);
    }
    
    Eigen::Matrix4f CBP_MatrixUtil::makeProjectionMatrix(float fov,
                                                         float aspect,
                                                         float zNear,
                                                         float zFar,
                                                         bool leftHanded) {
        Eigen::Matrix4f projectionMatrix = Eigen::Matrix4f::Zero();
        
        float f = 1.0f/tanf(fov * 0.5f);
        
        if (leftHanded) {
            aspect *= -1.0f;
        }
        
        projectionMatrix(0,0) = f / aspect; //0,0
        projectionMatrix(1,0) = 0.0f; //
        projectionMatrix(2,0) = 0.0f;
        projectionMatrix(3,0) = 0.0f;
        
        projectionMatrix(0,1) = 0.0f; //0,1
        projectionMatrix(1,1) = f; // 1,1
        projectionMatrix(2,1) = 0.0f; //2,1
        projectionMatrix(3,1) = 0.0f; //3,1
        
        projectionMatrix(0,2) = 0.0f; //0,2
        projectionMatrix(1,2) = 0.0f; // 1,2
        projectionMatrix(2,2) = (zFar+zNear) / (zNear-zFar); // 2,2
        projectionMatrix(3,2) = -1.0f; // 3,2
        
        projectionMatrix(0,3) = 0.0f; //0,3
        projectionMatrix(1,3) = 0.0f; //1,3
        projectionMatrix(2,3) = 2.0f * zFar * zNear /  (zNear-zFar); //2,3
        projectionMatrix(3,3) = 0.0f; //3,3
        
        return projectionMatrix;
    }
    
    Eigen::Matrix4f CBP_MatrixUtil::makeScaleMatrix(float x,float y,float z)
    {
        const float vals[] =
        {
            x,0,0,0,
            0,y,0,0,
            0,0,z,0,
            0,0,0,1
        };
        return Eigen::Matrix4f(vals);
    }
    
    //http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche92.html
    Eigen::Matrix4f CBP_MatrixUtil::makeProjectionMatrix(const Eigen::Matrix3f &cameraIntrinsics, float zn, float zf) {
        float fx = cameraIntrinsics(0,0);
        float fy = cameraIntrinsics(1,1);
        float wdiv2 = cameraIntrinsics(2,0);
        float hdiv2 = cameraIntrinsics(2,1);
        
        const float vals[] =
        {
            fx/wdiv2,0,0,0,
            0,fy/hdiv2,0,0,
            0,0,-(zf+zn)/(zf-zn), - (2.0f * zf * zn)/(zf-zn),
            0,0,-1,0
        };
        return Eigen::Matrix4f(vals);
    }
    
    Eigen::Matrix3f CBP_MatrixUtil::normalMatrix(const Eigen::Matrix4f& in)
    {
        Eigen::Matrix3f  ret;
        for(int i=0;i<3;i++)
            for(int j=0;j<3;j++)
                ret(i,j)=in(i,j);
        return ret.transpose().inverse();
    }
    
    Eigen::Matrix4f CBP_MatrixUtil::makeTranslationMatrix(float x, float y, float z) {
        Eigen::Affine3f transform(Eigen::Translation3f(x, y, z));
        return transform.matrix();
    }
    
    Eigen::Matrix4f CBP_MatrixUtil::convertTo4x4Matrix(const Eigen::Matrix3f &matrix) {
        Eigen::Matrix4f newMatrix = Eigen::Matrix4f::Identity();
        newMatrix.block(0,0,3,3) = matrix;
        return newMatrix;
    }
    
    Eigen::Matrix3f CBP_MatrixUtil::convertTo3x3Matrix(const Eigen::Matrix4f &matrix) {
        return matrix.block<3,3>(0,0);
    }
    
    cv::Mat_<float> CBP_MatrixUtil::getCameraMatrix(cv::Size frameSize, float HFOV)
    {
#if 1
        // dim = dimensioni immagine finale
        float cx = float(frameSize.width) / 2.0f;
        float cy = float(frameSize.height) / 2.0f;
        
        float VFOV = ((HFOV)/cx)*cy;
        
        float fx = fabs(float(frameSize.width) / (2 * tanf(HFOV)));
        float fy = fabs(float(frameSize.height) / (2 * tanf(VFOV)));
        
#else
        float f = 4.1;
        float resX = (float)(frameSize.width);
        float resY = (float)(frameSize.height);
        float sensorSizeX = 4.89;
        float sensorSizeY = 3.67;
        float fx = f * resX / sensorSizeX;
        float fy = f * resY / sensorSizeY;
        float cx = resX/2.;
        float cy = resY/2.;
#endif
        
        cv::Mat_<float> X = (cv::Mat_<float>(3, 3) <<
                              fx, 0, cx,
                              0, fy, cy,
                              0, 0, 1);
        
        return X;
    }
    
    Eigen::Matrix3f CBP_MatrixUtil::makeCameraMatrix(cv::Size frameSize, float HFOV) {
        Eigen::Matrix3f eigen;
        
        cv::cv2eigen(getCameraMatrix(frameSize, HFOV), eigen);
        
        return eigen;
    }

    // Converts a given Rotation Matrix to Euler angles
    cv::Mat CBP_MatrixUtil::rot2euler(const cv::Mat & rotationMatrix)
    {
        cv::Mat euler(3,1,CV_64F);
        
        double m00 = rotationMatrix.at<double>(0,0);
        double m02 = rotationMatrix.at<double>(0,2);
        double m10 = rotationMatrix.at<double>(1,0);
        double m11 = rotationMatrix.at<double>(1,1);
        double m12 = rotationMatrix.at<double>(1,2);
        double m20 = rotationMatrix.at<double>(2,0);
        double m22 = rotationMatrix.at<double>(2,2);
        
        double x, y, z;
        
        // Assuming the angles are in radians.
        if (m10 > 0.998) { // singularity at north pole
            x = 0;
            y = CV_PI/2;
            z = atan2(m02,m22);
        }
        else if (m10 < -0.998) { // singularity at south pole
            x = 0;
            y = -CV_PI/2;
            z = atan2(m02,m22);
        }
        else
        {
            x = atan2(-m12,m11);
            y = asin(m10);
            z = atan2(-m20,m00);
        }
        
        euler.at<double>(0) = x;
        euler.at<double>(1) = y;
        euler.at<double>(2) = z;
        
        return euler;
    }
    
    // Converts a given Euler angles to Rotation Matrix
    cv::Mat CBP_MatrixUtil::euler2rot(const cv::Mat & euler)
    {
        cv::Mat rotationMatrix(3,3,CV_64F);
        
        double x = euler.at<double>(0);
        double y = euler.at<double>(1);
        double z = euler.at<double>(2);
        
        // Assuming the angles are in radians.
        double ch = cos(z);
        double sh = sin(z);
        double ca = cos(y);
        double sa = sin(y);
        double cb = cos(x);
        double sb = sin(x);
        
        double m00, m01, m02, m10, m11, m12, m20, m21, m22;
        
        m00 = ch * ca;
        m01 = sh*sb - ch*sa*cb;
        m02 = ch*sa*sb + sh*cb;
        m10 = sa;
        m11 = ca*cb;
        m12 = -ca*sb;
        m20 = -sh*ca;
        m21 = sh*sa*cb + ch*sb;
        m22 = -sh*sa*sb + ch*cb;
        
        rotationMatrix.at<double>(0,0) = m00;
        rotationMatrix.at<double>(0,1) = m01;
        rotationMatrix.at<double>(0,2) = m02;
        rotationMatrix.at<double>(1,0) = m10;
        rotationMatrix.at<double>(1,1) = m11;
        rotationMatrix.at<double>(1,2) = m12;
        rotationMatrix.at<double>(2,0) = m20;
        rotationMatrix.at<double>(2,1) = m21;
        rotationMatrix.at<double>(2,2) = m22;
        
        return rotationMatrix;
    }
    
//    cv::Point3f projectPixelTo3dRay(const cv::Point2f& uv_rect, const cv::Size &frameSize, float HFOV)
//    {
//        // dim = dimensioni immagine finale
//        float cx = float(frameSize.width) / 2.0f;
//        float cy = float(frameSize.height) / 2.0f;
//        
//        float VFOV = ((HFOV)/cx)*cy;
//        
//        float fx = abs(float(frameSize.width) / (2 * tanf(HFOV)));
//        float fy = abs(float(frameSize.height) / (2 * tanf(VFOV)));
//        
//        float tx = 0;
//        float ty = 0;
//        
//        cv::Point3d ray;
//        ray.x = (uv_rect.x - cx - tx) / fx;
//        ray.y = (uv_rect.y - cy - ty) / fy;
//        ray.z = 1.0;
//        return ray;
//    }
    
    Eigen::ParametrizedLine<float,3> CBP_MatrixUtil::projectPixelTo3DRay(const cv::Point2f& uv, const cv::Size &frameSize, float HFOV) {
        // dim = dimensioni immagine finale
        float cx = float(frameSize.width) / 2.0f;
        float cy = float(frameSize.height) / 2.0f;
        
        float VFOV = ((HFOV)/cx)*cy;
        
        float fx = fabs(float(frameSize.width) / (2 * tanf(HFOV)));
        float fy = fabs(float(frameSize.height) / (2 * tanf(VFOV)));
        
        float tx = 0;
        float ty = 0;
        
        cv::Point3d ray;
        ray.x = (uv.x - cx - tx) / fx;
        ray.y = (uv.y - cy - ty) / fy;
        ray.z = 1.0;

        
        Eigen::ParametrizedLine<float,3> u(Eigen::Vector3f(uv.x, uv.y, 0.0),
                                           Eigen::Vector3f((uv.x - cx - tx) / fx, (uv.y - cy - ty) / fy, 1.0));
        return u;
    }
    
//    bool getPlaneIntersection(const Eigen::Vector3f &normal,
//                              const Eigen::Vector3f &p0,
//                              const Eigen::Vector3f &l0,
//                              const Eigen::Vector3f &l, float &t)
//    {
//        // assuming vectors are all normalized
//        
//        float denom = cv::dotProduct(normal, l);
//        if (denom > 1e-6) {
//            Vec3f p0l0 = p0 - l0;
//            t = dotProduct(p0l0, normal) / denom;
//            return (t >= 0);
//        }
//        
//        return false; 
//    }
//    
    //graph points online: http://technology.cpm.org/general/3dgraph/
    bool CBP_MatrixUtil::getPlaneIntersection(const cv::Point2f &point2D,
                              const cv::Size &viewSize,
                              const cv::Size_<float> &fov,
                              const Eigen::Hyperplane<float, 3> &hyperplane,
                              const Eigen::Matrix4f &mvpInverted,
                              float maxDistanceThreshold,
                              Eigen::Vector3f &intersection)
    {

        //Eigen::Vector4f nearPoint(0, 0, 0, 1);
        Eigen::Vector4f farPoint(point2D.x/(float)viewSize.width * 2.0 - 1.0,
                                 point2D.y/(float)viewSize.height * 2.0 - 1.0, 0.0, 1.0);
        
        Eigen::Vector4f fRotated = mvpInverted * farPoint;
        
        float factor = 1.0f;// / fRotated.w();
        
        Eigen::Vector3f origin(0, 0, 0);
        Eigen::Vector3f farRotated = Eigen::Vector3f(fRotated.x(), fRotated.y(), fRotated.z());
        
        Eigen::ParametrizedLine<float,3> ray(origin, farRotated);
        
        //Eigen::ParametrizedLine<float,3> ray = projectPixelTo3DRay(position2f, viewSize, hfov);
        
        intersection = factor * ray.intersectionPoint(hyperplane);
        

        //int viewport[] = {0, 0, viewSize.width, viewSize.height};
        //Transformation of normalized coordinates between -1 and 1
        //        in[0]=(winx-(float)viewport[0])/(float)viewport[2]*2.0-1.0;
        //        in[1]=(winy-(float)viewport[1])/(float)viewport[3]*2.0-1.0;
        //        in[2]=2.0*winz-1.0;
        //        in[3]=1.0;
        //        //Objects coordinates
        //        MultiplyMatrixByVector4by4OpenGL_FLOAT(out, m, in);
        //        if(out[3]==0.0)
        //            return 0;
        //        out[3]=1.0/out[3];
        //        objectCoordinate[0]=out[0]*out[3];
        //        objectCoordinate[1]=out[1]*out[3];
        //        objectCoordinate[2]=out[2]*out[3];
        
        if (intersection.norm() > maxDistanceThreshold) {
            return false;//probably should look at error, but anyway, too far away, maybe pa
        }
        
        float angle = acosf(farRotated.normalized().dot(intersection.normalized()));
        
        //point3D = cv::Point3f(intersection.x(), intersection.y(), intersection.z());
        bool isForward = angle < fov.width;
        
        return isForward;
    }
    
    //http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
    Eigen::Vector3f CBP_MatrixUtil::getEulerAngles(const Eigen::Quaternionf &quat) {
        
        double sqw = quat.w() * quat.w();
        double sqx = quat.x() * quat.x();
        double sqy = quat.y() * quat.y();
        double sqz = quat.z() * quat.z();
        double unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
        double test = quat.x() * quat.y() + quat.z() * quat.w();
        
        Eigen::Vector3f result;
        if (test > 0.48f * unit) { // singularity at north pole
            result[0] = 2 * atan2(quat.x(),quat.w());
            result[1] = M_PI/2;
            result[2] = 0;
            return result;
        }
        if (test < -0.48f * unit) { // singularity at south pole
            result[0] = -2.0f * atan2(quat.x(),quat.w());
            result[1] = -M_PI/2.0f;
            result[2] = 0;
            return result;
        }
        result[0] = atan2(2.0f * quat.y() * quat.w()- 2.0f * quat.x() * quat.z() , sqx - sqy - sqz + sqw);
        result[1] = asin(2.0f * test/unit);
        result[2] = atan2(2.0f * quat.x() * quat.w() - 2.0f * quat.y() * quat.z() , -sqx + sqy - sqz + sqw);
        
        return result;
    }
    
    Eigen::Vector3f CBP_MatrixUtil::getEulerAngles(Eigen::Matrix3f &rotation) {
        return rotation.eulerAngles(0, 1, 2);
    }
    
    double CBP_MatrixUtil::angleBetweenVectors(const Eigen::Vector3f &vectorA, const Eigen::Vector3f &vectorB) {
        double dot = vectorA.dot(vectorB);
        dot = ( dot < -1.0 ? -1.0 : ( dot > 1.0 ? 1.0 : dot ) );
        return fabs(acos( dot ));
    }

//    void tangentVector(const cv::Point3f &v1, const cv::Point3f &v2, const cv::Point3f &v3,
//                       const cv::Point2f &w1, const cv::Point2f &w2, const cv::Point2f &w3) {
//        
//        float x1 = v2.x - v1.x;
//        float x2 = v3.x - v1.x;
//        float y1 = v2.y - v1.y;
//        float y2 = v3.y - v1.y;
//        float z1 = v2.z - v1.z;
//        float z2 = v3.z - v1.z;
//        
//        float s1 = w2.x - w1.x;
//        float s2 = w3.x - w1.x;
//        float t1 = w2.y - w1.y;
//        float t2 = w3.y - w1.y;
//        
//        float r = 1.0F / (s1 * t2 - s2 * t1);
//        Eigen::Vector3f sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
//                      (t2 * z1 - t1 * z2) * r);
//        Eigen::Vector3f tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
//                      (s1 * z2 - s2 * z1) * r);
//        
//        Eigen::Vector3f tan1(sdir, sdir, sdir);
//
//        
//        Eigen::Vector3f tan2;
//        tan2[i1] += tdir;
//        tan2[i2] += tdir;
//        tan2[i3] += tdir;
//        
//        triangle++;
//    }
}
