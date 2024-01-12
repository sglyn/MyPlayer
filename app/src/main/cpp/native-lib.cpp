#include <jni.h>
#include <string>

#include "Log.h"
#include "JavaCaller.h"
#include "Player.h"


extern "C" {
#include <libavutil/avutil.h>
}

JavaVM *javaVm = 0;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVm = vm;
    return JNI_VERSION_1_4;
}


extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_myplayer_Player_nativeInit(JNIEnv *env, jobject thiz) {

    JavaCaller *javaCaller = new JavaCaller(javaVm, env, thiz);
    Player *player = new Player(javaCaller);

    return (jlong) player;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myplayer_Player_nativeSetPath(JNIEnv *env,
                                               jobject thiz,
                                               jlong handle,
                                               jstring _path) {
    const char *path = env->GetStringUTFChars(_path, 0);

    Player *player = (Player *) handle;
    player->setPath(path);

    env->ReleaseStringUTFChars(_path, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myplayer_Player_nativePrepare(JNIEnv *env, jobject thiz, jlong handle) {
    LOGE("Player::nativePrepared()");

    Player *player = (Player *) handle;

    player->prepare();


}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_myplayer_Player_nativePlay(JNIEnv *env, jobject thiz, jlong handle) {
    LOGE("Player::nativePlay()");
    Player *player = (Player *) handle;

    player->play();
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_myplayer_Player_nativeSetSurface(JNIEnv *env, jobject thiz, jlong handle,
                                                  jobject surface) {

    LOGE("Player::nativeSetSurface()");
    Player *player = (Player *) handle;

    ANativeWindow* window = ANativeWindow_fromSurface(env,surface);
    player->setWindow(window);
}