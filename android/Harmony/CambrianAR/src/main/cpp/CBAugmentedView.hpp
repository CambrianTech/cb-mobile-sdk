//
// Created by Joel Teply on 8/3/17.
//
#include <jni.h>
#include <string>
#include <cbar/CBAR_View.hpp>
#include <memory>
#include "CBJavaObject.hpp"

namespace cbjni {

    class CBAugmentedScene;

    struct CBARCallback : cbar::CBAR_CallbackI {

        CBARCallback(jobject context);

        virtual void historyChanged(std::shared_ptr<cbpipe::UndoState> undo, bool forward);

        virtual void colorsCallback(std::vector<cbar::ColorInfo>&colors);

        virtual void exposureCallback(cv::Point2f exposurePoint);

        virtual void focusCallback(cv::Point2f focusPoint);

        jobject m_instance = 0;
    };

    class CBAugmentedView : public CBJavaObject {
    public:
        CBAugmentedView(JNIEnv* env, jobject instance);
        ~CBAugmentedView();

        std::shared_ptr<cbar::CBAR_View> getCoreView();

        static const char* getJavaClassPath() { return "Lcom/cambrian/cbar/CBAugmentedView"; }

        static CBAugmentedView * getObject(JNIEnv* env, jobject instance) {
            return CBJavaObject::getObject<CBAugmentedView>(env, instance);
        };

        void setCoreScene(CBAugmentedScene *scene);

        virtual void touchedAt(float normalizedX, float normalizedY, TouchStep step);
        virtual void rotatedBy(float amount);
        virtual cv::Scalar getColorInVideoAtPoint(cv::Point2f point);
    protected:
        std::shared_ptr<CBARCallback> getCallback();

        virtual std::shared_ptr<cbar::CBAR_View> generateCoreView(std::shared_ptr<CBARCallback> callback);

    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };


}
