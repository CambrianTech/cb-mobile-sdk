//
// Created by Joel Teply on 8/9/17.
//

#include <cbar/scene/CBAR_RemodelingScene.hpp>
#include <cbar/scene/CBAR_Paint.hpp>
#include "CBPaintAsset.hpp"
#include "../CBUtils.hpp"

using namespace cbar;
using namespace cbpipe;
using namespace cbscene;
using namespace cbjni;

namespace cbjni {

    struct CBPaintAsset::Impl {
        Impl(CBPaintAsset *parent) : m_parent(parent) {
            CBLog("Finished creating CBPaintAsset '%s'", parent->getAssetID().c_str());
        }

        ~Impl() {

        }

        CBPaintAsset *m_parent;
    };

    CBPaintAsset::CBPaintAsset(JNIEnv *env, jobject instance, std::string assetID) : CBAugmentedAsset(env, instance, assetID) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }

    CBPaintAsset::~CBPaintAsset() {

    }

    std::shared_ptr<cbscene::CBAR_MaskedAsset> CBPaintAsset::generateCoreAsset(jobject instance, std::string assetID) {
        return std::shared_ptr<cbscene::CBAR_Paint>(new cbscene::CBAR_Paint(assetID, getCallback()));
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingPaint_createNativeInstance(JNIEnv *env,
                                                                         jobject instance,
                                                                         jstring assetID_) {
    const char *assetID = env->GetStringUTFChars(assetID_, 0);

    jlong objectPtr = (jlong) new cbjni::CBPaintAsset(env, instance, assetID);

    env->ReleaseStringUTFChars(assetID_, assetID);

    return objectPtr;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingPaint_getColor(JNIEnv *env, jobject instance) {

    auto *asset = cbjni::CBPaintAsset::getObject(env, instance);
    if (!asset || !asset->getCorePaint()) return 0;

    auto paint = asset->getCorePaint();

    return intColorForScalar(paint->getColor());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingPaint_setColor(JNIEnv *env, jobject instance,
                                                             jint color) {

    auto *asset = cbjni::CBPaintAsset::getObject(env, instance);
    if (!asset || !asset->getCorePaint()) return;

    auto paint = asset->getCorePaint();

    paint->setColor(scalarForIntColor(color));
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingPaint__1getTransparency(JNIEnv *env,
                                                                      jobject instance) {
    auto *asset = cbjni::CBPaintAsset::getObject(env, instance);
    if (!asset || !asset->getCorePaint()) return 0;

    auto paint = asset->getCorePaint();

    return jint(paint->getTransparency());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingPaint__1setTransparency(JNIEnv *env, jobject instance,
                                                                      jint transparency) {

    auto *asset = cbjni::CBPaintAsset::getObject(env, instance);
    if (!asset || !asset->getCorePaint()) return;

    auto paint = asset->getCorePaint();

    paint->setTransparency((Transparency) transparency);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingPaint__1getSheen(JNIEnv *env, jobject instance) {

    auto *asset = cbjni::CBPaintAsset::getObject(env, instance);
    if (!asset || !asset->getCorePaint()) return 0;

    auto paint = asset->getCorePaint();

    return jint(paint->getSheen());

}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingPaint__1setSheen(JNIEnv *env, jobject instance,
                                                               jint value) {

    auto *asset = cbjni::CBPaintAsset::getObject(env, instance);
    if (!asset || !asset->getCorePaint()) return;

    auto paint = asset->getCorePaint();

    paint->setSheen((Sheen) value);
}