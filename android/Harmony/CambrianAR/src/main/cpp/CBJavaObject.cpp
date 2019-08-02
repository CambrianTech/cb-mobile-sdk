//
// Created by Joel Teply on 8/8/17.
//

#include <jni.h>
#include <utility/Diagnostics.h>
#include "CBJavaObject.hpp"

static JavaVM *m_jvm = 0;

namespace cbjni {
    struct CBJavaObject::Impl {
        Impl(CBJavaObject *parent, JNIEnv* env, jobject instance) : m_parent(parent) {
            if (!m_jvm) {
                jint rs = env->GetJavaVM(&m_jvm);
                assert (rs == JNI_OK);
            }
            m_instance = env->NewGlobalRef(instance);
        }

        ~Impl() {
            JNIEnv *env;
            jint rs = m_jvm->AttachCurrentThread(&env, NULL);
            assert (rs == JNI_OK);
            env->DeleteGlobalRef(m_instance);
        }

        CBJavaObject *m_parent;
        jobject m_instance;
    };

    CBJavaObject::CBJavaObject(JNIEnv* env, jobject instance) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, env, instance));
    }

    CBJavaObject::~CBJavaObject() {

    }

    jobject CBJavaObject::getJavaInstance() {
        return m_pImpl->m_instance;
    }

    CBJavaObject *CBJavaObject::_getObject(JNIEnv *env, jobject instance) {
        jclass objclass = env->GetObjectClass(instance);
        jmethodID method = env->GetMethodID(objclass, "getCoreObject", "()J");

        jlong corePtr = (jlong) env->CallLongMethod(instance, method);

        return (CBJavaObject *) corePtr;
    }

    JNIEnv *CBJavaObject::getJavaEnv() {
        JNIEnv *env;
        jint rs = m_jvm->AttachCurrentThread(&env, NULL);
        assert (rs == JNI_OK);
        return env;
    }
};
