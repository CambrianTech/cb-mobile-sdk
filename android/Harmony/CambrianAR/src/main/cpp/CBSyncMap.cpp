//
// Created by Joel Teply on 8/9/17.
//
#include <string>

#include <jni.h>
#include <memory>
#include <map>
#include <utility/Diagnostics.h>
#include <cbar/scene/CBAR_Asset.hpp>
#include "CBUtils.hpp"
#include "CBAugmentedScene.hpp"
#include "CBAugmentedAsset.hpp"

template <typename T> std::map<std::string, T> * getStdMap(jlong mapPtr) {
    return (std::map<std::string, T> *) mapPtr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBStringSyncMap_put(JNIEnv *env, jobject instance, jlong mapPtr,
                                                     jstring key_, jstring value_) {
    const char *key = env->GetStringUTFChars(key_, 0);
    const char *value = env->GetStringUTFChars(value_, 0);

    std::map<std::string, std::string> *map = getStdMap<std::string>(mapPtr);

    if (!map) return;

    (*map)[key] = std::string(value);

    env->ReleaseStringUTFChars(key_, key);
    env->ReleaseStringUTFChars(value_, value);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBStringSyncMap_remove(JNIEnv *env, jobject instance, jlong mapPtr,
                                                        jstring key_) {
    const char *key = env->GetStringUTFChars(key_, 0);

    std::map<std::string, std::string> *map = getStdMap<std::string>(mapPtr);

    map->erase(key);

    env->ReleaseStringUTFChars(key_, key);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBStringSyncMap_clear(JNIEnv *env, jobject instance,
                                                       jlong mapPtr) {

    std::map<std::string, std::string> *map = getStdMap<std::string>(mapPtr);
    map->clear();

}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedAssetSyncMap_put(JNIEnv *env, jobject instance, jlong scenePtr,
                                                   jstring key_, jobject jasset) {
    //const char *key = env->GetStringUTFChars(key_, 0);

    cbjni::CBAugmentedScene *scene = (cbjni::CBAugmentedScene *)scenePtr;
    cbjni::CBAugmentedAsset *asset = cbjni::CBAugmentedAsset::getObject(env, jasset);

    if (!asset || !asset->getCoreAsset() || !scene || !scene->getCoreScene()) return;

    scene->getCoreScene()->appendAsset(asset->getCoreAsset());

    //env->ReleaseStringUTFChars(key_, key);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedAssetSyncMap_remove(JNIEnv *env, jobject instance,
                                                                jlong scenePtr, jstring key_) {
    const char *key = env->GetStringUTFChars(key_, 0);

    cbjni::CBAugmentedScene *scene = (cbjni::CBAugmentedScene *) scenePtr;

    scene->getCoreScene()->removeAsset(std::string(key));

    env->ReleaseStringUTFChars(key_, key);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedAssetSyncMap_clear(JNIEnv *env, jobject instance,
                                                               jlong scenePtr) {
    cbjni::CBAugmentedScene *scene = (cbjni::CBAugmentedScene *)scenePtr;

    scene->getCoreScene()->clearAssets();
}