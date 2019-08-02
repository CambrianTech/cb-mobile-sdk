//
// Created by Joel Teply on 8/9/17.
//

#include "CBAugmentedAsset.hpp"

using namespace cbar;
using namespace cbpipe;
using namespace cbscene;
using namespace cbjni;

namespace cbjni {

    struct CBAugmentedAsset::Impl {
        Impl(CBAugmentedAsset *parent, jobject instance, const std::string &assetID)
                : m_parent(parent), m_instance(instance), m_assetID(assetID) {

            m_callback = std::shared_ptr<CBARAssetCallback>(new CBARAssetCallback(parent));
        }

        ~Impl() {

        }

        CBAugmentedAsset *m_parent;
        std::string m_assetID;
        jobject m_instance;
        std::shared_ptr<cbscene::CBAR_MaskedAsset> m_coreAsset;
        std::shared_ptr<CBARAssetCallback> m_callback;
    };

    CBAugmentedAsset::CBAugmentedAsset(JNIEnv *env, jobject instance, std::string assetID) : CBJavaObject(env, instance) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, instance, assetID));
    }

    CBAugmentedAsset::~CBAugmentedAsset() {

    }

    std::shared_ptr<CBAR_MaskedAsset> CBAugmentedAsset::getCoreAsset() {
        if (!m_pImpl->m_coreAsset) {
            m_pImpl->m_coreAsset = generateCoreAsset(m_pImpl->m_instance, m_pImpl->m_assetID);
        }
        return m_pImpl->m_coreAsset;
    }

    const std::string &CBAugmentedAsset::getAssetID() {
        return getCoreAsset()->getAssetID();
    }

    std::shared_ptr<CBARAssetCallback> CBAugmentedAsset::getCallback() {
        return m_pImpl->m_callback;
    }

    void CBARAssetCallback::attachedToScene() {
        //[getAsset() attachedToScene];
    }

    void CBARAssetCallback::detachedFromScene() {
        //[getAsset() detachedFromScene];
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedAsset_destroyNativeInstance(JNIEnv *env,
                                                                        jobject instance,
                                                                        jlong p_native_ptr) {

    if( p_native_ptr )
        delete (cbjni::CBAugmentedAsset*)p_native_ptr;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_cambrian_cbar_CBAugmentedAsset_getAssetID(JNIEnv *env, jobject instance) {
    auto *augmentedAsset = cbjni::CBAugmentedAsset::getObject(env, instance);
    return env->NewStringUTF(augmentedAsset->getCoreAsset()->getAssetID().c_str());
}
