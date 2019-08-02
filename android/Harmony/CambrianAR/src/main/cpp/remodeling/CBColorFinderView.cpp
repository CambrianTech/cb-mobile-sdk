//
// Created by Joel Teply on 8/9/17.
//

#include <cbar/scene/CBAR_RemodelingScene.hpp>
#include <cbar/scene/CBAR_Paint.hpp>
#include "CBColorFinderView.hpp"
#include "../CBUtils.hpp"

using namespace cbar;
using namespace cbpipe;
using namespace cbscene;
using namespace cbjni;

namespace cbjni {

    struct CBColorFinderView::Impl {
        Impl(CBColorFinderView *parent) : m_parent(parent) {}

        ~Impl() {}

        CBColorFinderView *m_parent;
    };

    CBColorFinderView::CBColorFinderView(JNIEnv *env, jobject instance)
            : CBAugmentedView(env, instance) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }

    CBColorFinderView::~CBColorFinderView() {

    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_cambrian_cbar_remodeling_CBColorFinderView_createNativeInstance(JNIEnv *env,
                                                                         jobject instance) {

    auto *view = new cbjni::CBColorFinderView(env, instance);
    view->getCoreView()->setToolMode(ToolModeFindColor);

    return jlong(view);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_cambrian_cbar_remodeling_CBColorFinderView__1getColorAtPoint(JNIEnv *env,
                                                                      jobject instance,
                                                                      jfloat x,
                                                                      jfloat y) {

    auto *augmentedView = cbjni::CBAugmentedView::getObject(env, instance);
    if (!augmentedView || !augmentedView->getCoreView() || !augmentedView->getCoreView()->getScene()) return 0;


    auto point = cv::Point2f(x, y);

    auto color = new cv::Scalar();

    auto scalar = augmentedView->getCoreView()->getColorInVideoAtPoint(point);
    return intColorForScalar(scalar);

}