//
// Created by Joel Teply on 8/11/17.
//

#include "CBGLRenderer.hpp"
#include "CBAugmentedView.hpp"

#include <utility/Diagnostics.h>
#include <cbar/CBAR_GLOffscreenRenderer.hpp>

#include <bgfx/platform.h>

#include <stdio.h>
#include <bx/thread.h>
#include <bx/timer.h>

#include <android/input.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

using namespace cbjni;
using namespace cbar;

namespace cbjni {

    struct CBGLRenderer::Impl {
        Impl(CBGLRenderer *parent, CBAugmentedView *view) : m_parent(parent), m_view(view) {}

        ~Impl() {}

        CBGLRenderer *m_parent;
        CBAugmentedView *m_view;
        ANativeWindow *m_window;

        size_t m_frameWidth, m_frameHeight;
        int m_outputWidth, m_outputHeight;
        int m_rotation = 0;
        float m_hfov;

        CBAR_GLOffscreenRenderer m_offscreenRenderer;

        GLuint m_offscreenTextureID = 0;

        void prepareViewport(JNIEnv *env, jobject surface,
                             jint frameWidth,  jint frameHeight,
                             jint outputWidth, jint outputHeight,
                             jfloat deviceFOV, jint rotation) {

            m_window = ANativeWindow_fromSurface (env, surface);

            m_frameWidth = size_t(frameWidth);
            m_frameHeight = size_t(frameHeight);
            m_outputWidth = outputWidth;
            m_outputHeight = outputHeight;
            m_rotation = rotation;
            m_hfov = float(deviceFOV);


            m_view->getCoreView()->prepareViewport(m_window, nullptr,
                                                   m_frameWidth, m_frameHeight,
                                                   m_outputWidth, m_outputHeight,
                                                   m_hfov, m_rotation);

            m_offscreenTextureID = m_offscreenRenderer.create(GLuint(frameWidth), GLuint(frameHeight), GL_TEXTURE_EXTERNAL_OES);

            m_view->getCoreView()->startRunning();
        }

        void stillRender() {
            if (!m_view || !m_view->getCoreView()) return;
            m_view->getCoreView()->stillRender();
        }

        void addFrame(int textureID, int rotation,
                      uint8_t *dataY, size_t dataYLength,
                      uint8_t *dataU, size_t dataULength,
                      uint8_t *dataV, size_t dataVLength) {

            if (!m_view || !m_view->getCoreView()) return;

            if (m_offscreenTextureID) {
                m_offscreenRenderer.render(GLuint(textureID));
            }

            cbar::RawFrame frame;

            size_t dataLength = dataYLength + dataULength + dataVLength;
            uint8_t *bytes = new uint8_t[dataLength];

            memcpy(bytes, dataY, dataYLength);
            memcpy(bytes + dataYLength, dataU, dataULength);
            memcpy(bytes + dataYLength + dataULength, dataV, dataVLength);

            frame.frameType = frame_type_YUV_420_888;
            frame.data = bytes;
            frame.width = m_frameWidth;
            frame.height = m_frameHeight;
            frame.textureID = m_offscreenTextureID;
            frame.stride = m_frameWidth;
            frame.dataLength = dataLength;
            frame.frameRotation = rotation;

            m_view->getCoreView()->addFrame(frame);

            delete[] bytes;
        }

        void destroyViewport() {

        }
    };

    CBGLRenderer::CBGLRenderer(JNIEnv *env, jobject instance, jobject _view) : CBJavaObject(env, instance) {
        CBAugmentedView *view = CBAugmentedView::getObject(env, _view);
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, view));
    }

    CBGLRenderer::~CBGLRenderer() {

    }

    void CBGLRenderer::prepareViewport(JNIEnv *env, jobject surface,
                                       jint frameWidth,  jint frameHeight,
                                       jint outputWidth, jint outputHeight,
                                       jfloat deviceFOV, jint rotation) {

        m_pImpl->prepareViewport(env, surface,
                                 frameWidth,  frameHeight,
                                 outputWidth, outputHeight,
                                 deviceFOV, rotation);

    }

    void CBGLRenderer::destroyViewport() {
        m_pImpl->destroyViewport();
    }

    void CBGLRenderer::stillRender() {
        m_pImpl->stillRender();
    }

    void CBGLRenderer::addFrame(int textureID, int rotation,
                                jbyte *dataY, jsize dataYLength,
                                jbyte *dataU, jsize dataULength,
                                jbyte *dataV, jsize dataVLength) {
        m_pImpl->addFrame(textureID, rotation,
                          (uint8_t *)dataY, (size_t) dataYLength,
                          (uint8_t *)dataU, (size_t) dataULength,
                          (uint8_t *)dataV, (size_t) dataVLength);
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_cambrian_cbar_CBGLRenderer_createNativeInstance(JNIEnv *env, jobject instance,
                                                         jobject view) {
    jlong objectPtr = (jlong) new cbjni::CBGLRenderer(env, instance, view);
    return objectPtr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBGLRenderer_destroyNativeInstance(JNIEnv *, jobject,
                                                         jlong p_native_ptr) {

    if( p_native_ptr )
        delete (cbjni::CBGLRenderer*)p_native_ptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBGLRenderer__1prepareViewport(JNIEnv *env, jobject instance,
                                                      jobject surface, jint frameWidth,
                                                      jint frameHeight, jint outputWidth,
                                                      jint outputHeight, jfloat deviceFOV,
                                                      jint frameRotation) {

    auto *renderer = CBGLRenderer::getObject(env, instance);
    if (!renderer) return;

    renderer->prepareViewport(env, surface,
                              frameWidth,  frameHeight,
                              outputWidth, outputHeight,
                              deviceFOV, frameRotation);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBGLRenderer_destroyViewport(JNIEnv *env, jobject instance) {

    auto *bgfxEntry = CBGLRenderer::getObject(env, instance);
    if (!bgfxEntry) return;

    bgfxEntry->destroyViewport();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBGLRenderer_stillRender(JNIEnv *env, jobject instance) {

    auto *bgfxEntry = CBGLRenderer::getObject(env, instance);
    if (!bgfxEntry) return;

    bgfxEntry->stillRender();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_cambrian_cbar_CBGLRenderer_addFrame(JNIEnv *env, jobject instance, jint textureID, jint rotation,
                                             jbyteArray dataY_, jbyteArray dataU_, jbyteArray dataV_) {

    auto *bgfxEntry = CBGLRenderer::getObject(env, instance);
    if (!bgfxEntry) return;

    jbyte *dataY = env->GetByteArrayElements(dataY_, NULL);
    jsize dataYlength = env->GetArrayLength(dataY_);

    jbyte *dataU = env->GetByteArrayElements(dataU_, NULL);
    jsize dataUlength = env->GetArrayLength(dataU_);

    jbyte *dataV = env->GetByteArrayElements(dataV_, NULL);
    jsize dataVlength = env->GetArrayLength(dataV_);

    bgfxEntry->addFrame(int(textureID), int(rotation), dataY, dataYlength, dataU, dataUlength, dataV, dataVlength);

    env->ReleaseByteArrayElements(dataY_, dataY, 0);
    env->ReleaseByteArrayElements(dataU_, dataU, 0);
    env->ReleaseByteArrayElements(dataV_, dataV, 0);
}
