
#ifndef __Cambrian__CBUtils__
#define __Cambrian__CBUtils__

#include <jni.h>
#include <imaging/Coloring.h>

int intColorForScalar(cv::Scalar color);

cv::Scalar scalarForIntColor(jint hexColor);

cv::Scalar scalarConvertBgrToRgb(cv::Scalar src);

jobject stdMapToJava(JNIEnv* env, const std::map<std::string, std::string> &map);

#endif /* defined(__Cambrian__CBUtils__) */