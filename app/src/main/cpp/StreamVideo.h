//
// Created by sg on 2024/1/11.
//

#ifndef MYPLAYER_STREAMVIDEO_H
#define MYPLAYER_STREAMVIDEO_H


#include "Stream.h"
#include "android/native_window_jni.h"


class StreamVideo : public Stream {

public:
    StreamVideo(JavaCaller *javaCaller,
                AVFormatContext* avFormatContext,
                AVCodecContext *avCodecContext,
                AVStream *avStream);

    virtual ~StreamVideo();

    virtual void actualPlay();
    virtual void actualStop();

    void handeDecodeSpeed();

    void calcFps();

    double fps;

    ANativeWindow* window = 0;

    ANativeWindow *getWindow() const;

    void setWindow(ANativeWindow *window);

    pthread_mutex_t surfaceMutex;

    double syc(double delay);

    void draw(uint8_t *pString[4], int pInt[4]);
};




#endif //MYPLAYER_STREAMVIDEO_H
