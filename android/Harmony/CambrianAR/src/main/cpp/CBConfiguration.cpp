//
// Created by Joel Teply on 8/3/17.
//
#include <jni.h>
#include <string>
#include <utility/Diagnostics.h>
#include <cbar/CBAR_Common.hpp>


extern "C" {

    cbar::CBConfig config;

    JNIEXPORT void JNICALL
    Java_com_cambrian_cbar_CBConfiguration_setCBAssetPath(JNIEnv *env, jobject instance,
                                                          jstring path_) {
        const char *path = env->GetStringUTFChars(path_, 0);

        config.cbAssetPath = path;

        env->ReleaseStringUTFChars(path_, path);
    }

    JNIEXPORT void JNICALL
    Java_com_cambrian_cbar_CBConfiguration_setWorkingPath(JNIEnv *env, jobject instance,
                                                          jstring path_) {
        const char *path = env->GetStringUTFChars(path_, 0);

        config.workingPath = path;

        env->ReleaseStringUTFChars(path_, path);
    }

    JNIEXPORT void JNICALL
    Java_com_cambrian_cbar_CBConfiguration_setCBLoggingPath(JNIEnv *env, jobject instance,
                                                            jstring path_) {
        const char *path = env->GetStringUTFChars(path_, 0);

        config.loggingPath = path;

        env->ReleaseStringUTFChars(path_, path);
    }

    JNIEXPORT void JNICALL
    Java_com_cambrian_cbar_CBConfiguration_initialize(JNIEnv *env, jobject instance) {

        cbar::CB_Initialize(config);

    }

    JNIEXPORT void JNICALL
    Java_com_cambrian_cbar_remodeling_CBRemodelingConfiguration_setPrimaryAssetPath(JNIEnv *env,
                                                                                    jobject instance,
                                                                                    jstring path_) {
        const char *path = env->GetStringUTFChars(path_, 0);

        config.primaryAssetPath = path;

        env->ReleaseStringUTFChars(path_, path);
    }
    JNIEXPORT void JNICALL
    Java_com_cambrian_cbar_remodeling_CBRemodelingConfiguration_setSecondaryAssetPath(JNIEnv *env,
                                                                                      jobject instance,
                                                                                      jstring path_) {
        const char *path = env->GetStringUTFChars(path_, 0);

        config.secondaryAssetPath = path;

        env->ReleaseStringUTFChars(path_, path);
    }
}
