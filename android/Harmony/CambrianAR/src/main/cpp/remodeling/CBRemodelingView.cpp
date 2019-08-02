//
// Created by Joel Teply on 8/9/17.
//

#include <cbar/scene/CBAR_RemodelingScene.hpp>
#include <cbar/scene/CBAR_Paint.hpp>
#include "CBRemodelingView.hpp"

using namespace cbar;
using namespace cbpipe;
using namespace cbscene;
using namespace cbjni;

namespace cbjni {

    struct CBRemodelingView::Impl {
        Impl(CBRemodelingView *parent) : m_parent(parent) {}

        ~Impl() {}

        CBRemodelingView *m_parent;
    };

    CBRemodelingView::CBRemodelingView(JNIEnv *env, jobject instance)
            : CBAugmentedView(env, instance) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }

    CBRemodelingView::~CBRemodelingView() {

    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_cambrian_cbar_remodeling_CBRemodelingView_createNativeInstance(JNIEnv *env,
                                                                        jobject instance) {

    auto *view = new cbjni::CBRemodelingView(env, instance);
    view->getCoreView()->setToolMode(ToolModeFill);

    return jlong (view);
}