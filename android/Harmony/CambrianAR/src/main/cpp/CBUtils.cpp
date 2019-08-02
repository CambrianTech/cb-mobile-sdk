//
// Created by Joel Teply on 8/9/17.
//

//
//  CBUtils.h
//  Cambrian
//
//  Created by Joel Teply on 7/11/13.
//
//


#include "CBUtils.hpp"
#include <jni.h>
#include <iostream>
#include <imaging/Imaging.h>
#include <imaging/ImageProcessing.h>

jint intColorForScalar(cv::Scalar color)
{
    //convert to hex
    int hexColor = 0;
    hexColor |= int(color[0]) << 16;
    hexColor |= int(color[1]) << 8;
    hexColor |= int(color[2]);
    hexColor |= int(color[3]) << 24;

    //CBLog("Generated %i from (%f, %f, %f, %f)", hexColor, color[0], color[1], color[2], color[3]);
    return hexColor;

}

cv::Scalar scalarForIntColor(jint hexColor)
{
    //convert
    cv::Scalar color;
    color[0] = (hexColor & 0x00FF0000) >> 16;
    color[1] = (hexColor & 0x0000FF00) >> 8;
    color[2] = (hexColor & 0x000000FF);
    color[3] = (hexColor & 0xFF000000) >> 24;

    return color;
}

jobject stdMapToJava(JNIEnv *env, const std::map<std::string, std::string> &map) {

    jclass mapClass = env->FindClass("java/util/HashMap");

    jmethodID init = env->GetMethodID(mapClass, "<init>", "(I)V");
    jobject hashMap = env->NewObject(mapClass, init, map.size());

    jmethodID putMethod = env->GetMethodID(mapClass, "put",
                                           "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    for ( const auto &it : map) {
        jstring key = env->NewStringUTF( it.first.c_str() );
        jstring value = env->NewStringUTF( it.second.c_str());

        env->CallVoidMethod(
                hashMap,
                putMethod,
                key,
                value
        );
    }

    env->DeleteLocalRef(mapClass);

    return hashMap;
}

