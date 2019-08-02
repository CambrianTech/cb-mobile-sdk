//
// Created by Joel Teply on 8/3/17.
//
#include <jni.h>
#include <string>
#include <imaging/Coloring.h>
#include "CBUtils.hpp"

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_cambrian_cbar_CBColoring_getComplementsForColor(JNIEnv *env, jclass type,
                                                                   jint color, jint count,
                                                                   jint angleSpan) {

    // TODO

}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_cambrian_cbar_CBColoring_getAdjacentColors(JNIEnv *env, jclass type, jint color,
                                                              jint count, jint angleSpan) {

    // TODO

}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_cambrian_cbar_CBColoring_getShadesOfColor(JNIEnv *env, jclass type, jint color,
                                                             jint count) {

    // TODO

}
extern "C"
JNIEXPORT jdouble JNICALL
Java_com_cambrian_cbar_CBColoring_getEuclideanDistance__II(JNIEnv *env, jclass type,
                                                                     jint colorA, jint colorB) {

    // TODO
    double distance = imaging::Coloring::euclideanDistance(scalarForIntColor(colorA), scalarForIntColor(colorB));

    return distance;
}

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_cambrian_cbar_CBColoring_getEuclideanDistance__IIZLfloat_3_093_2(JNIEnv *env,
                                                                                    jclass type,
                                                                                    jint colorA,
                                                                                    jint colorB,
                                                                                    jboolean asHSV,
                                                                                    jfloatArray coeficient_) {
    jfloat *coeficient = env->GetFloatArrayElements(coeficient_, NULL);

    // TODO

    env->ReleaseFloatArrayElements(coeficient_, coeficient, 0);
}
extern "C"
JNIEXPORT jdouble JNICALL
Java_com_cambrian_cbar_CBColoring_getHumanPerceptiveDistance(JNIEnv *env, jclass type,
                                                                       jint colorA, jint colorB) {

    double distance = imaging::Coloring::perceptiveDistance(scalarForIntColor(colorA), scalarForIntColor(colorB));

    return distance;
}