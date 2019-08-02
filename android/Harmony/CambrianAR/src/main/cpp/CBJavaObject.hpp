//
// Created by Joel Teply on 8/8/17.
//

#ifndef HARMONY_CBJAVAOBJECT_H
#define HARMONY_CBJAVAOBJECT_H

#include <jni.h>
#include <string>
#include <memory>

namespace cbjni {

    class CBJavaObject {
    public:

        CBJavaObject(JNIEnv* env, jobject instance);

        jobject getJavaInstance();

        static std::string getJavaClassPath() { return ""; };

        static JNIEnv *getJavaEnv();

    protected:
        virtual ~CBJavaObject();

        template <typename T>
        static T* getObject(JNIEnv* env, jobject instance) {
            return (T*) _getObject(env, instance);
        }
    private:
        static CBJavaObject* _getObject(JNIEnv* env, jobject instance);

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif //HARMONY_CBJAVAOBJECT_H
