//
// Created by Joel Teply on 8/3/17.
//
#include <jni.h>
#include <string>
#include <android/log.h>
#include <utility/Diagnostics.h>

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_cambrian_cbar_CBLicensing_isEnabled(JNIEnv *env, jclass type) {

    CBLog("isEnabled called");

    return (jboolean) true;
}

JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBLicensing_enableWithKey(JNIEnv *env, jclass type, jstring key_) {
    const char *key = env->GetStringUTFChars(key_, 0);

    // TODO

    env->ReleaseStringUTFChars(key_, key);
}

}