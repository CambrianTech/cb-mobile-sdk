//
// Created by Joel Teply on 8/9/17.
//

#include <cbar/scene/CBAR_RemodelingScene.hpp>
#include <cbar/scene/CBAR_Floor.hpp>
#include "CBRemodelingScene.hpp"

using namespace cbar;
using namespace cbpipe;
using namespace cbscene;
using namespace cbjni;

namespace cbjni {

    struct CBRemodelingScene::Impl {
        Impl(CBRemodelingScene *parent) : m_parent(parent) {
            CBLog("Finished creating CBRemodelingScene");
        }

        ~Impl() { }

        std::shared_ptr<cbscene::CBAR_Scene> generateCoreScene(generate_asset_fn genAssetFn,
                                                                         const std::string &sceneID,
                                                                         jobject instance) {

            return std::shared_ptr<cbscene::CBAR_Scene>(new cbscene::CBAR_RemodelingScene(genAssetFn, sceneID, instance));
        }

        std::shared_ptr<cbscene::CBAR_Scene> generateCoreScene(generate_asset_fn genAssetFn,
                                                                         const cv::Mat &rgbImage,
                                                                         const std::string &sceneID,
                                                                         jobject instance) {

            return std::shared_ptr<cbscene::CBAR_Scene>(new cbscene::CBAR_RemodelingScene(genAssetFn, rgbImage, sceneID, instance));
        }

        CBRemodelingScene *m_parent;
    };

    CBRemodelingScene::CBRemodelingScene(JNIEnv *env, jobject instance, std::string assetID)
            : CBAugmentedScene(env, instance, assetID) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }

    CBRemodelingScene::~CBRemodelingScene() {}

    std::shared_ptr<cbscene::CBAR_Scene>
    CBRemodelingScene::generateCoreScene(jobject instance, std::string sceneID) {
        return m_pImpl->generateCoreScene(genAssetFunction(), sceneID, instance);
    }

    std::shared_ptr<cbscene::CBAR_Scene>
    CBRemodelingScene::generateCoreScene(jobject instance, cv::Mat rgbImage, std::string sceneID) {
        return m_pImpl->generateCoreScene(genAssetFunction(), rgbImage, sceneID, instance);
    }
}

extern "C"
JNIEXPORT jlongArray JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingScene_initWithID(JNIEnv *env,
                                                                         jobject instance,
                                                                         jstring sceneID_) {
    jlongArray ret = env->NewLongArray(2);
    jlong *pointers = env->GetLongArrayElements(ret, 0);

    const char *sceneID = env->GetStringUTFChars(sceneID_, 0);

    CBRemodelingScene *scene = new cbjni::CBRemodelingScene(env, instance, sceneID);
    pointers[0] = (jlong) scene;

    env->ReleaseStringUTFChars(sceneID_, sceneID);

    pointers[1] = (jlong) &(scene->getCoreScene()->getUserData());

    env->ReleaseLongArrayElements(ret, pointers, NULL);

    return ret;
}


