//
// Created by sg on 2024/1/10.
//

#ifndef MYPLAYER_JAVACALLER_H
#define MYPLAYER_JAVACALLER_H

#include "H.h"
#include <jni.h>
#include "Log.h"





class JavaCaller {
private:
    JavaVM *javaVm = 0;
    JNIEnv *env = 0;
    jobject javaPlayerObj;

    jmethodID onErrorMethod;
    jmethodID onPreparedMethod;

public:

    JavaCaller(JavaVM *_javaVm,JNIEnv *_env,jobject& _player);

    virtual ~JavaCaller();

    void onError(ErrorCode code,Thread thread = THREAD_MAIN );
    void onPrepared(int thread = THREAD_MAIN);
};


#endif //MYPLAYER_JAVACALLER_H
