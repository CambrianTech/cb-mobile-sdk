//
// Created by Joel Teply on 8/3/17.
//

#include <string>

#include "CBAugmentedView.hpp"
#include "CBAugmentedScene.hpp"
#include "CBUtils.hpp"

#include <jni.h>
#include <cbar/CBAR_View.hpp>
#include <cbar/CBAR_Common.hpp>
#include <utility/Directory.h>
#include <memory>

using namespace cbar;
using namespace cbpipe;

namespace cbjni {

    struct CBAugmentedView::Impl {
        Impl(CBAugmentedView *parent, JNIEnv* env, jobject instance)
                : m_parent(parent) {

            m_callback = std::shared_ptr<CBARCallback>(new CBARCallback(parent->getJavaInstance()));
        }

        ~Impl() { }

        void setCoreScene(CBAugmentedScene *scene) {
            m_coreView->setScene(scene->getCoreScene());
        }

        CBAugmentedView *m_parent;

        std::shared_ptr<CBARCallback> m_callback = 0;
        std::shared_ptr<cbar::CBAR_View> m_coreView = 0;
    };

#pragma mark CBAugmentedView implementation

    CBAugmentedView::CBAugmentedView(JNIEnv* env, jobject instance) : CBJavaObject(env, instance) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, env, instance));
    }

    CBAugmentedView::~CBAugmentedView() { }

    std::shared_ptr<cbar::CBAR_View> CBAugmentedView::generateCoreView(std::shared_ptr<CBARCallback> callback) {
        auto coreView = std::shared_ptr<CBAR_View>(new CBAR_View(callback));
        coreView->setToolMode(ToolModeFindColor);
        return coreView;
    }

    std::shared_ptr<cbar::CBAR_View> CBAugmentedView::getCoreView() {
        if (!m_pImpl->m_coreView) {
            m_pImpl->m_coreView = generateCoreView(getCallback());
        }
        return m_pImpl->m_coreView;
    }

    std::shared_ptr<CBARCallback> CBAugmentedView::getCallback() {
        return m_pImpl->m_callback;
    }

    void CBAugmentedView::setCoreScene(CBAugmentedScene *scene) {
        m_pImpl->setCoreScene(scene);
    }

    void CBAugmentedView::touchedAt(float normalizedX, float normalizedY, TouchStep step) {
        getCoreView()->touchedAt(cv::Point2f(normalizedX, normalizedY), step);
    }

    void CBAugmentedView::rotatedBy(float amount) {
        getCoreView()->rotatedBy(amount);
    }

    cv::Scalar CBAugmentedView::getColorInVideoAtPoint(cv::Point2f point) {
        return getCoreView()->getColorInVideoAtPoint(point);
    }

#pragma mark callbacks

    CBARCallback::CBARCallback(jobject instance) : cbar::CBAR_CallbackI(instance) {
        m_instance = instance;
    }

    void CBARCallback::historyChanged(std::shared_ptr<cbpipe::UndoState> undo, bool forward) {

//        JNIEnv *env = CBAugmentedView::getJavaEnv();
//
//        jclass classRef = env->FindClass("com/cambrian/cbar/CBAugmentedView");
//        jmethodID method = env->GetMethodID(classRef, "historyChanged", "(Ljava/lang/String;IZ)V");
//
//        jstring assetID = env->NewStringUTF(undo.assetID.c_str());
//        auto change = undo.change;
//        int changeNum = change;
//
//
//
//        env->CallVoidMethod(m_instance, method, assetID, changeNum, forward);
//
//        env->ReleaseStringUTFChars(assetID, undo.assetID.c_str());
    }

    void CBARCallback::colorsCallback(std::vector<cbar::ColorInfo> &colors) {
        JNIEnv *env = CBAugmentedView::getJavaEnv();

        int size = colors.size();
        jintArray jcolors = env->NewIntArray(size);
        jfloatArray jposx = env->NewFloatArray(size);
        jfloatArray jposy = env->NewFloatArray(size);

        jint color[size];
        jfloat posx[size];
        jfloat posy[size];

        for(int i = 0; i < size; i++) {
            float x = (colors[i].normalizedPosition.x);
            float y = (colors[i].normalizedPosition.y);
            int col = intColorForScalar(colors[i].color);
            //CBLog("color is %i, pos is %f, %f", col, x, y);

            color[i] = col;
            posx[i] = x;
            posy[i] = y;
        }

        env->SetIntArrayRegion(jcolors, 0, size, color);
        env->SetFloatArrayRegion(jposx, 0, size, posx);
        env->SetFloatArrayRegion(jposy, 0, size, posy);

        jclass classRef = env->GetObjectClass(m_instance);
        jmethodID method = env->GetMethodID(classRef, "colorsFound", "([I[F[F)V");
        env->CallVoidMethod(m_instance, method, jcolors, jposx, jposy);

        //env->ReleaseIntArrayElements(jcolors, color, 0);
        //env->ReleaseFloatArrayElements(jposx, posx, 0);
        //env->ReleaseFloatArrayElements(jposy, posy, 0);
    }

    void CBARCallback::exposureCallback(cv::Point2f exposurePoint) {}
    void CBARCallback::focusCallback(cv::Point2f focusPoint) {}
}


extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_destroyNativeInstance(JNIEnv *env,
                                                                       jobject instance,
                                                                       jlong p_native_ptr) {
    if( p_native_ptr ) {
        auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
        delete augmentedView;
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_CBAugmentedView_getUndoSize(JNIEnv *env, jobject instance) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return 0;

    return augmentedView->getCoreView()->getScene()->getUndoSize();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_CBAugmentedView_getMaxUndoSize(JNIEnv *env, jobject instance) {
    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return 0;

    return augmentedView->getCoreView()->getScene()->getMaxUndoSize();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_setMaxUndoSize(JNIEnv *env, jobject instance,
                                                                jint size) {
    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->getScene()->setMaxUndoSize(size);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_CBAugmentedView_getRedoSize(JNIEnv *env, jobject instance) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return 0;

    return augmentedView->getCoreView()->getScene()->getRedoSize();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_CBAugmentedView_getMaxRedoSize(JNIEnv *env, jobject instance) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return 0;

    return augmentedView->getCoreView()->getScene()->getMaxRedoSize();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_setMaxRedoSize(JNIEnv *env, jobject instance,
                                                                jint size) {
    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->getScene()->setMaxRedoSize(size);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_undo(JNIEnv *env, jobject instance) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->getScene()->undoState();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_redo(JNIEnv *env, jobject instance) {
    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->getScene()->redoState();
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_CBAugmentedView__1getToolMode(JNIEnv *env, jobject instance) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return 0;

    return augmentedView->getCoreView()->getToolMode();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView__1setToolMode(JNIEnv *env, jobject instance,
                                                               jint toolMode) {
    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->setToolMode((ToolMode)toolMode);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_clearAll(JNIEnv *env, jobject instance) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->clearAll();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView__1pause(JNIEnv *env, jobject instance, jboolean pause) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->pause(pause);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView__1startRunning(JNIEnv *env, jobject instance) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->startRunning();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView__1stopRunning(JNIEnv *env, jobject instance) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->stopRunning();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView__1captureToStill(JNIEnv *env, jobject instance) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return;

    augmentedView->getCoreView()->captureCurrentState();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_setCoreScene(JNIEnv *env, jobject instance,
                                                              jlong coreScenePtr) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView()) return;

    augmentedView->setCoreScene((cbjni::CBAugmentedScene *)coreScenePtr);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_touchedAt(JNIEnv *env, jobject instance,
                                                 jfloat normalizedX, jfloat normalizedY, jint step) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView()) return;

    augmentedView->touchedAt(float(normalizedX), float(normalizedY), TouchStep(step) );
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_rotatedBy(JNIEnv *env, jobject instance, jfloat amount) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView()) return;

    augmentedView->rotatedBy(amount);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBAugmentedView_setWorldTransform(JNIEnv *env, jobject instance,
                                                         jfloatArray quaternion_) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView()) return;


    jfloat *quaternion = env->GetFloatArrayElements(quaternion_, NULL);

    cv::Vec4f wxyz = cv::Vec4f(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);

    augmentedView->getCoreView()->setWorldTransform(wxyz, cv::Vec4f(0.0f, 0.0f, 13.5f, 1.0f));

    env->ReleaseFloatArrayElements(quaternion_, quaternion, 0);
}