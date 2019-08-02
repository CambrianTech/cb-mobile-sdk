//
// Created by Joel Teply on 8/9/17.
//

#ifndef HARMONY_CBAUGMENTEDASSET_H
#define HARMONY_CBAUGMENTEDASSET_H


#include <jni.h>
#include <string>
#include <jni.h>
#include <cbar/scene/scene.h>
#include <memory>
#include "CBJavaObject.hpp"

namespace cbjni {

    class CBAugmentedAsset;

    struct CBARAssetCallback : cbscene::CBAR_MaskedAssetCallbackI {

        CBARAssetCallback(CBAugmentedAsset *asset) : m_asset(asset) {

        }

        virtual void attachedToScene();

        virtual void detachedFromScene();

        CBAugmentedAsset* getAsset() {
            return m_asset;
        }

    private:
        CBAugmentedAsset *m_asset;
    };

    class CBAugmentedAsset : public CBJavaObject {
    public:
        CBAugmentedAsset(JNIEnv *env, jobject instance, std::string assetID);
        ~CBAugmentedAsset();

        static CBAugmentedAsset * getObject(JNIEnv* env, jobject instance) {
            return CBJavaObject::getObject<CBAugmentedAsset>(env, instance);
        };

        static const char* getJavaClassPath() { return "Lcom/cambrian/cbar/CBAugmentedAsset"; }

        std::shared_ptr<cbscene::CBAR_MaskedAsset> getCoreAsset();

        std::shared_ptr<CBARAssetCallback> getCallback();

        const std::string &getAssetID();

    protected:
        virtual std::shared_ptr<cbscene::CBAR_MaskedAsset> generateCoreAsset(jobject instance, std::string assetID) = 0;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

}


#endif //HARMONY_CBAUGMENTEDASSET_H
