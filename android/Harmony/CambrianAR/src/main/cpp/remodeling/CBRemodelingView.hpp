//
// Created by Joel Teply on 8/9/17.
//

#ifndef HARMONY_CBREMODELINGVIEW_H
#define HARMONY_CBREMODELINGVIEW_H

#include <jni.h>
#include <string>
#include <jni.h>
#include <cbar/scene/CBAR_Scene.hpp>
#include <memory>
#include "../CBAugmentedView.hpp"

namespace cbjni {

    class CBRemodelingView : public CBAugmentedView {
    public:
        CBRemodelingView(JNIEnv* env, jobject instance);
        ~CBRemodelingView();

        static CBRemodelingView * getObject(JNIEnv* env, jobject instance) {
            return CBJavaObject::getObject<CBRemodelingView>(env, instance);
        };

        static const char* getJavaClassPath() { return "Lcom/cambrian/cbar/remodeling/CBRemodelingView"; }

    protected:
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

}


#endif //HARMONY_CBAUGMENTEDASSET_H
