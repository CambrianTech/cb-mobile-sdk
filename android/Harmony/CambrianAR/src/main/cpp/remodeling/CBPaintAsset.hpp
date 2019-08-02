//
// Created by Joel Teply on 8/9/17.
//

#ifndef HARMONY_CBPAINTASSET_H
#define HARMONY_CBPAINTASSET_H


#include <jni.h>
#include <string>
#include <jni.h>
#include <cbar/scene/CBAR_Scene.hpp>
#include <memory>
#include "../CBAugmentedAsset.hpp"

namespace cbjni {

    class CBPaintAsset : public CBAugmentedAsset {
    public:
        CBPaintAsset(JNIEnv *env, jobject instance, std::string assetID);
        ~CBPaintAsset();

        static CBPaintAsset * getObject(JNIEnv* env, jobject instance) {
            return CBJavaObject::getObject<CBPaintAsset>(env, instance);
        };

        static const char* getJavaClassPath() { return "Lcom/cambrian/cbar/remodeling/CBPaintAsset"; }

        std::shared_ptr<cbscene::CBAR_Paint> getCorePaint() { return std::dynamic_pointer_cast<cbscene::CBAR_Paint>(getCoreAsset()); };

    protected:
        virtual std::shared_ptr<cbscene::CBAR_MaskedAsset> generateCoreAsset(jobject instance, std::string assetID);
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

}


#endif //HARMONY_CBAUGMENTEDASSET_H
