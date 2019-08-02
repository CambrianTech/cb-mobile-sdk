//
// Created by Joel Teply on 8/3/17.
//
#include <jni.h>
#include <string>
#include <jni.h>
#include <cbar/scene/CBAR_Scene.hpp>
#include <memory>
#include "CBJavaObject.hpp"
#include "CBAugmentedAsset.hpp"

namespace cbjni {

    class CBAugmentedScene : public CBJavaObject {
    public:
        CBAugmentedScene(JNIEnv* env, jobject instance, std::string sceneID);
        CBAugmentedScene(JNIEnv *env, jobject instance, cv::Mat rgbImage);

        CBAugmentedScene(JNIEnv *env, jobject instance, cv::Mat rgbImage, string sceneID);

        ~CBAugmentedScene();

        std::shared_ptr<cbscene::CBAR_Scene> getCoreScene();

        static CBAugmentedScene * getObject(JNIEnv* env, jobject instance) {
            return CBJavaObject::getObject<CBAugmentedScene>(env, instance);
        };

        static const char* getJavaClassPath() { return "Lcom/cambrian/cbar/CBAugmentedScene"; }

    protected:
        virtual std::shared_ptr<cbscene::CBAR_Scene> generateCoreScene(jobject instance, std::string sceneID);
        virtual std::shared_ptr<cbscene::CBAR_Scene> generateCoreScene(jobject instance, cv::Mat rgbImage, std::string sceneID);
        virtual cbscene::generate_asset_fn genAssetFunction();
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

}
