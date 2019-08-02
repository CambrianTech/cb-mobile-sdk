//
// Created by Joel Teply on 8/3/17.
//

#include <string>

#include "CBAugmentedScene.hpp"
#include "CBUtils.hpp"
#include <utility/Diagnostics.h>

#include <jni.h>
#include <cbar/scene/CBAR_Scene.hpp>
#include <memory>

using namespace cbar;
using namespace cbpipe;
using namespace cbscene;
using namespace cbjni;

namespace cbjni {

    struct CBAugmentedScene::Impl {
        Impl(CBAugmentedScene *parent, jobject instance, std::string sceneID)
                : m_parent(parent), m_instance(instance), m_sceneID(sceneID) {

        }

        Impl(CBAugmentedScene *parent, jobject instance, cv::Mat rgbImage, std::string sceneID)
                : m_parent(parent), m_instance(instance), m_rgbImage(rgbImage), m_sceneID(sceneID) {
        }

        ~Impl() { }

        CBAugmentedScene *m_parent;
        jobject m_instance;
        std::string m_sceneID;
        cv::Mat m_rgbImage;

        std::shared_ptr<CBAR_Scene> m_coreScene;
    };

#pragma mark CBAugmentedView implementation

    CBAugmentedScene::CBAugmentedScene(JNIEnv *env, jobject instance, cv::Mat rgbImage, std::string sceneID)
            : CBJavaObject(env, instance) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, instance, rgbImage, sceneID));
    }

    CBAugmentedScene::CBAugmentedScene(JNIEnv *env, jobject instance, std::string sceneID)
            : CBJavaObject(env, instance) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, instance, sceneID));
    }

    CBAugmentedScene::~CBAugmentedScene() {
        CBLog("Destroying CBAugmentedScene");
    }

    std::shared_ptr<CBAR_Scene> CBAugmentedScene::generateCoreScene(jobject instance, std::string sceneID) {
        auto coreScene = std::shared_ptr<cbscene::CBAR_Scene>(new cbscene::CBAR_Scene(genAssetFunction(), sceneID, instance));
        return coreScene;
    }

    std::shared_ptr<CBAR_Scene> CBAugmentedScene::generateCoreScene(jobject instance, cv::Mat rgbImage, std::string sceneID) {
        auto coreScene = std::shared_ptr<cbscene::CBAR_Scene>(new cbscene::CBAR_Scene(genAssetFunction(), rgbImage, sceneID, instance));
        return coreScene;
    }


    std::shared_ptr<CBAR_Scene> CBAugmentedScene::getCoreScene() {
        if (!m_pImpl->m_coreScene) {
            if(!m_pImpl->m_rgbImage.empty()) {
                m_pImpl->m_coreScene = generateCoreScene(m_pImpl->m_instance, m_pImpl->m_rgbImage, m_pImpl->m_sceneID);
            } else {
                m_pImpl->m_coreScene = generateCoreScene(m_pImpl->m_instance, m_pImpl->m_sceneID);
            }
        }
        return m_pImpl->m_coreScene;
    }

    generate_asset_fn CBAugmentedScene::genAssetFunction() {
        jobject instance = getJavaInstance();

        return [instance](cbscene::CBAR_Asset::asset_type assetType, const std::string &assetID) {

            if (!instance) { CBError("No java instance of scene"); }

            //CBLog("Creating asset for AssetID=%s", assetID.c_str());
            JNIEnv *env = getJavaEnv();
            jstring id = env->NewStringUTF(assetID.c_str());

            jclass assetClass;
            if(assetType == cbscene::CBAR_Asset::asset_type_paint) {
                assetClass = env->FindClass("com/cambrian/cbar/remodeling/CBRemodelingPaint");
            } else if(assetType == cbscene::CBAR_Asset::asset_type_floor) {
                assetClass = env->FindClass("com/cambrian/cbar/remodeling/CBRemodelingFloor");
            }

            jmethodID constructor = env->GetMethodID(assetClass, "<init>", "(Ljava/lang/String;)V");
            jobject jasset = env->NewObject(assetClass, constructor, id);

            jclass sceneClass = env->FindClass("com/cambrian/cbar/CBAugmentedScene");
            jmethodID appendMethod = env->GetMethodID(sceneClass, "appendAsset", "(Lcom/cambrian/cbar/CBAugmentedAsset;)V");
            env->CallVoidMethod(instance, appendMethod, jasset);

            auto *asset = CBAugmentedAsset::getObject(env, jasset);
            auto coreAsset = asset->getCoreAsset();

            return coreAsset;
        };
    }
}

extern "C"
JNIEXPORT jlongArray JNICALL
Java_com_cambrian_cbar_CBAugmentedScene_initWithID(JNIEnv *env,
                                                   jobject instance,
                                                   jstring sceneID_) {

    jlongArray ret = env->NewLongArray(2);
    jlong *pointers = env->GetLongArrayElements(ret, 0);

    const char *sceneID = env->GetStringUTFChars(sceneID_, 0);

    CBAugmentedScene *scene = new cbjni::CBAugmentedScene(env, instance, sceneID);
    pointers[0] = (jlong) scene;

    env->ReleaseStringUTFChars(sceneID_, sceneID);

    pointers[1] = (jlong) &(scene->getCoreScene()->getUserData());

    env->ReleaseLongArrayElements(ret, pointers, NULL);

    return ret;
}

extern "C"
JNIEXPORT jlongArray JNICALL
Java_com_cambrian_cbar_CBAugmentedScene_initWithImagePath(JNIEnv *env, jobject instance,
                                                          jstring sceneID_, jstring path_) {
    const char *sceneID = env->GetStringUTFChars(sceneID_, 0);
    const char *path = env->GetStringUTFChars(path_, 0);

    jlongArray ret = env->NewLongArray(2);
    jlong *pointers = env->GetLongArrayElements(ret, 0);

    cv::Mat img_ = cv::imread(path);


    int width = img_.size().width;
    CBLog("image width is %i", width);
    float ratio = (1080 / double(width));
    CBLog("ratio is %f", ratio);
    cv::Mat image;
    cv::resize(img_, image, cv::Size(), ratio, ratio);

    if(image.channels() == 3) {
        cv::cvtColor(image, image, CV_RGB2RGBA);
    }

    CBAugmentedScene *scene = new cbjni::CBAugmentedScene(env, instance, image, sceneID);
    pointers[0] = (jlong) scene;

    env->ReleaseStringUTFChars(sceneID_, sceneID);
    env->ReleaseStringUTFChars(path_, path);

    pointers[1] = (jlong) &(scene->getCoreScene()->getUserData());

    env->ReleaseLongArrayElements(ret, pointers, NULL);

    return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedScene_destroyNativeInstance(JNIEnv *env,
                                                                        jobject instance,
                                                                        jlong p_native_ptr) {

    if( p_native_ptr )
        delete (cbjni::CBAugmentedScene*)p_native_ptr;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_cambrian_cbar_CBAugmentedScene_getSceneID(JNIEnv *env, jobject instance) {

    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return nullptr;

    return env->NewStringUTF(augmentedScene->getCoreScene()->getSceneID().c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedScene_loadFromDirectory(JNIEnv *env, jobject instance,
                                                                    jstring path_) {
    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return;

    const char *path = env->GetStringUTFChars(path_, 0);

    augmentedScene->getCoreScene()->loadFromDirectory(path, 0);

    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedScene__1saveToDirectory(JNIEnv *env, jobject instance,
                                                                    jstring path_,
                                                                    jboolean doCompression) {
    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return;

    const char *path = env->GetStringUTFChars(path_, 0);

    augmentedScene->getCoreScene()->saveToDirectory(path, doCompression);

    env->ReleaseStringUTFChars(path_, path);

    //return env->NewStringUTF(finalPath.c_str());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_CBAugmentedScene__1getLightingAdjustment(JNIEnv *env,
                                                                          jobject instance) {
    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return 0;

    return (jint) augmentedScene->getCoreScene()->getLighting();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedScene__1setLightingAdjustment(JNIEnv *env,
                                                                          jobject instance,
                                                                          jint adjustment) {
    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return;

    augmentedScene->getCoreScene()->setLighting((LightingType) adjustment);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_cambrian_cbar_CBAugmentedScene_getSelectedAssetID(JNIEnv *env,
                                                                     jobject instance) {

    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return 0;

    auto assetID = augmentedScene->getCoreScene()->getSelectedAssetID();

    return assetID.length() ? env->NewStringUTF(assetID.c_str()) : nullptr;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_cambrian_cbar_CBAugmentedScene_setSelectedAssetID(JNIEnv *env,
                                                           jobject instance, jstring id) {

    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return 0;

    const char *assetID = env->GetStringUTFChars(id, 0);

    return (jboolean) augmentedScene->getCoreScene()->setSelectedAssetID(assetID);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedScene_setSelectedAsset(JNIEnv *env, jobject instance, jobject asset) {
    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return;

    auto *augmentedAsset = cbjni::CBAugmentedAsset::getObject(env, asset);

    augmentedScene->getCoreScene()->setSelectedAsset(augmentedAsset->getCoreAsset());
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_cambrian_cbar_CBAugmentedScene__1getFrameInfo(JNIEnv *env, jobject instance) {

    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return 0;

    auto frame = augmentedScene->getCoreScene()->getVideoFrame();

    if (!frame) {
        CBError("NO FRAME");
        return 0;
    }


    auto frameSize = frame->frameSize();

    int size = 3;
    jintArray ret = env->NewIntArray(size);
    jint frameData[size];

    frameData[0] = frameSize.width;
    frameData[1] = frameSize.height;
    frameData[2] = frame->getFrameRotation();

    env->SetIntArrayRegion(ret, 0, size, frameData);

    CBLog("Frame size width: %i", frameSize.width);
    CBLog("Frame size height: %i", frameSize.height);

    return ret;

}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_cambrian_cbar_CBAugmentedScene_hasWorldTransform(JNIEnv *env, jobject instance) {

    auto *augmentedScene = cbjni::CBAugmentedScene::getObject(env, instance);
    if (!augmentedScene || !augmentedScene->getCoreScene()) return 0;

    return (jboolean) augmentedScene->getCoreScene()->hasWorldTransform();
}