//
// Created by Joel Teply on 8/11/17.
//

#ifndef HARMONY_CBGLRENDERER_H
#define HARMONY_CBGLRENDERER_H

#include <jni.h>
#include <string>
#include <jni.h>
#include <memory>
#include "CBJavaObject.hpp"

namespace cbjni {

    class CBGLRenderer : public CBJavaObject {
    public:
        CBGLRenderer(JNIEnv *env, jobject instance, jobject view);

        ~CBGLRenderer();

        static CBGLRenderer *getObject(JNIEnv *env, jobject instance) {
            return CBJavaObject::getObject<CBGLRenderer>(env, instance);
        };

        static const char *getJavaClassPath() { return "Lcom/cambrian/cbar/CBGLRenderer"; }

        void prepareViewport(JNIEnv *env, jobject surface,
                             jint frameWidth,  jint frameHeight,
                             jint outputWidth, jint outputHeight,
                             jfloat deviceFOV, jint rotation);

        void destroyViewport();

        void stillRender();

        void addFrame(int textureID, int rotation,
                      jbyte *dataY, jsize dataYLength,
                      jbyte *dataU, jsize dataULength,
                      jbyte *dataV, jsize dataVLength);

    protected:


    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}


#endif //HARMONY_CBGLRENDERER_H
