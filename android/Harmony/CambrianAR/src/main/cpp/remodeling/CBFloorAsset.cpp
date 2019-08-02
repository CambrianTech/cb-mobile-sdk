//
// Created by Joel Teply on 8/9/17.
//

#include <cbar/scene/CBAR_RemodelingScene.hpp>
#include <cbar/scene/CBAR_Floor.hpp>
#include "CBFloorAsset.hpp"

using namespace cbar;
using namespace cbpipe;
using namespace cbscene;
using namespace cbjni;

namespace cbjni {

    struct CBFloorAsset::Impl {
        Impl(CBFloorAsset *parent) : m_parent(parent) {
            CBLog("Finished creating CBFloorAsset");
        }

        ~Impl() {

        }

        CBFloorAsset *m_parent;
    };

    CBFloorAsset::CBFloorAsset(JNIEnv *env, jobject instance, std::string assetID) : CBAugmentedAsset(env, instance, assetID) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }

    CBFloorAsset::~CBFloorAsset() {

    }

    std::shared_ptr<cbscene::CBAR_MaskedAsset>
    CBFloorAsset::generateCoreAsset(jobject instance, std::string assetID) {
        return std::shared_ptr<cbscene::CBAR_Floor>(new cbscene::CBAR_Floor(assetID, getCallback()));
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingFloor_createNativeInstance(JNIEnv *env,
                                                                         jobject instance,
                                                                         jstring assetID_) {
    const char *assetID = env->GetStringUTFChars(assetID_, 0);

    jlong objectPtr = (jlong) new cbjni::CBFloorAsset(env, instance, assetID);

    env->ReleaseStringUTFChars(assetID_, assetID);

    return objectPtr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingFloor_setDiffuse(JNIEnv *env, jobject instance,
                                                               jbyteArray diffuse_) {

    jbyte *diffuseArray = env->GetByteArrayElements(diffuse_, NULL);

    //cv::Mat diffuse(1024, 1024, CV_8UC3, diffuseArray);
    cv::Mat diffuse(1024, 1024, CV_8UC3, (unsigned char *)diffuseArray);
    diffuse = cv::imdecode(diffuse, CV_LOAD_IMAGE_COLOR);
    cv::cvtColor(diffuse, diffuse, CV_RGB2BGR);

    auto *floor = cbjni::CBFloorAsset::getObject(env, instance);
    if (!floor || !floor->getCoreFloor()) return;

    floor->getCoreFloor()->setDiffuse(diffuse);

    env->ReleaseByteArrayElements(diffuse_, diffuseArray, 0);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingFloor_setNormalRoughness(JNIEnv *env,
                                                                       jobject instance,
                                                                       jbyteArray normal_,
                                                                       jbyteArray roughness_) {


    jbyte *normalArray = env->GetByteArrayElements(normal_, NULL);
    jbyte *roughnessArray = env->GetByteArrayElements(roughness_, NULL);

    cv::Mat normal(1024, 1024, CV_8UC3, normalArray);
    cv::Mat roughness(1024, 1024, CV_8UC1, roughnessArray);

    normal = cv::imdecode(normal, CV_LOAD_IMAGE_COLOR);
    roughness = cv::imdecode(roughness, CV_LOAD_IMAGE_GRAYSCALE);


    auto *floor = cbjni::CBFloorAsset::getObject(env, instance);
    if (!floor || !floor->getCoreFloor()) return;

    floor->getCoreFloor()->setNormalRoughness(normal, roughness);

    env->ReleaseByteArrayElements(normal_, normalArray, 0);
    env->ReleaseByteArrayElements(roughness_, roughnessArray, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingFloor_setScale(JNIEnv *env, jobject instance,
                                                             jfloat scale) {

    auto *floor = cbjni::CBFloorAsset::getObject(env, instance);
    if (!floor || !floor->getCoreFloor()) return;

    floor->getCoreFloor()->setScale(scale);

}