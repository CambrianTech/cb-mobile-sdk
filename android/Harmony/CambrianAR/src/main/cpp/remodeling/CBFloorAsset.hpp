//
// Created by Joel Teply on 8/9/17.
//

#ifndef HARMONY_CBFLOORASSET_H
#define HARMONY_CBFLOORASSET_H


#include <jni.h>
#include <string>
#include <jni.h>
#include <cbar/scene/CBAR_Scene.hpp>
#include <memory>
#include "../CBAugmentedAsset.hpp"

namespace cbjni {

    class CBFloorAsset : public CBAugmentedAsset {
    public:
        CBFloorAsset(JNIEnv *env, jobject instance, std::string assetID);
        ~CBFloorAsset();

        static CBFloorAsset * getObject(JNIEnv* env, jobject instance) {
            return CBJavaObject::getObject<CBFloorAsset>(env, instance);
        };

        static const char* getJavaClassPath() { return "Lcom/cambrian/cbar/remodeling/CBFloorAsset"; }

        std::shared_ptr<cbscene::CBAR_Floor> getCoreFloor() { return std::dynamic_pointer_cast<cbscene::CBAR_Floor>(getCoreAsset()); };

    protected:
        virtual std::shared_ptr<cbscene::CBAR_MaskedAsset> generateCoreAsset(jobject instance, std::string assetID);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

}


#endif //HARMONY_CBAUGMENTEDASSET_H
