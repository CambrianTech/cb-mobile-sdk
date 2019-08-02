//
// Created by Joel Teply on 8/9/17.
//

#ifndef HARMONY_CBREMODELINGSCENE_H
#define HARMONY_CBREMODELINGSCENE_H

#include <jni.h>
#include <string>
#include <jni.h>
#include <cbar/scene/CBAR_Scene.hpp>
#include <memory>
#include "../CBAugmentedScene.hpp"

namespace cbjni {

    class CBRemodelingScene : public CBAugmentedScene {
    public:
        CBRemodelingScene(JNIEnv *env, jobject instance, std::string assetID);
        ~CBRemodelingScene();

        static CBRemodelingScene * getObject(JNIEnv* env, jobject instance) {
            return CBJavaObject::getObject<CBRemodelingScene>(env, instance);
        };

        static const char* getJavaClassPath() { return "Lcom/cambrian/cbar/remodeling/CBRemodelingScene"; }

    protected:

        virtual std::shared_ptr<cbscene::CBAR_Scene> generateCoreScene(jobject instance, std::string sceneID);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;

        shared_ptr<cbscene::CBAR_Scene>
    generateCoreScene(jobject instance, cv::Mat rbgImage, string sceneID);
    };

}


#endif //HARMONY_CBREMODELINGSCENE_H
