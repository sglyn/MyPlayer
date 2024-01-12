//
// Created by sg on 2024/1/10.
//

#include "JavaCaller.h"


JavaCaller::JavaCaller(JavaVM *_javaVm, JNIEnv *_env, jobject &_player)
    :javaVm(_javaVm),env(_env)
{
    javaPlayerObj = env->NewGlobalRef(_player);

    jclass clz = env->GetObjectClass(javaPlayerObj);

    onErrorMethod = env->GetMethodID(clz,"onError","(I)V");
    onPreparedMethod = env->GetMethodID(clz,"onPrepared","()V");

    LOGE("player=%d",javaPlayerObj);
}

JavaCaller::~JavaCaller() {
    env->DeleteGlobalRef(javaPlayerObj);
    javaPlayerObj = 0;
}

void JavaCaller::onError(ErrorCode code, Thread thread) {
    if(thread == THREAD_MAIN){
        env->CallVoidMethod(javaPlayerObj,onErrorMethod,code);
    } else {
        // 要用这个线程的jniEnv去调用 java端的方法，否则crash
        JNIEnv *jniEnv;
        jint attached = javaVm->AttachCurrentThread(&jniEnv,0);
        if(attached != JNI_OK){
            LOGE("JavaCaller::onError() attach thread failed");
            return;
        }
        jniEnv->CallVoidMethod(javaPlayerObj,onErrorMethod,code);

        javaVm->DetachCurrentThread();
    }
}

void JavaCaller::onPrepared(int thread) {
    if(thread == THREAD_MAIN){
        env->CallVoidMethod(javaPlayerObj,onPreparedMethod);
    } else {
        JNIEnv *jniEnv;
        jint attached = javaVm->AttachCurrentThread(&jniEnv,0);

        if(attached != JNI_OK){
            LOGE("JavaCaller::onPrepared() attach thread failed");
            return;
        }
        jniEnv->CallVoidMethod(javaPlayerObj,onPreparedMethod);
        javaVm->DetachCurrentThread();
    }
}
