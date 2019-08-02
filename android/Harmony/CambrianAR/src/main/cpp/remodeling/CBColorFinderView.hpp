//
// Created by Joel Teply on 8/9/17.
//

#ifndef HARMONY_CBCOLORFINDERVIEW_H
#define HARMONY_CBCOLORFINDERVIEW_H

#include <jni.h>
#include <string>
#include <jni.h>
#include <cbar/scene/CBAR_Scene.hpp>
#include <memory>
#include "../CBAugmentedView.hpp"

namespace cbjni {

    class CBColorFinderView : public CBAugmentedView {
    public:
        CBColorFinderView(JNIEnv* env, jobject instance);
        ~CBColorFinderView();

        static CBColorFinderView * getObject(JNIEnv* env, jobject instance) {
            return CBJavaObject::getObject<CBColorFinderView>(env, instance);
        };

        static const char* getJavaClassPath() { return "Lcom/cambrian/cbar/remodeling/CBColorFinderView"; }

    protected:
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };



}


#endif //HARMONY_CBCOLORFINDERVIEW_H
