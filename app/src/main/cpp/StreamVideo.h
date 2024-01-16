//
// Created by sg on 2024/1/11.
//

#ifndef MYPLAYER_STREAMVIDEO_H
#define MYPLAYER_STREAMVIDEO_H


#include "Stream.h"
#include "android/native_window_jni.h"


class StreamVideo : public Stream {
    friend void funHandeVideoDecodeSpeed(Stream *thiz);
public:
    StreamVideo(JavaCaller *javaCaller,
                AVFormatContext* avFormatContext,
                AVCodecContext *avCodecContext,
                AVStream *avStream);

    virtual ~StreamVideo();

    virtual void actualPlay();
    virtual void actualStop();

    ANativeWindow *getWindow() const;
    void setWindow(ANativeWindow *window);


private:
    double fps;
    ANativeWindow* window = 0;
    pthread_mutex_t surfaceMutex;

    double syc(double delay);
    void draw(uint8_t *pString[4], int pInt[4]);

    void handeDecodeSpeed();
    void calcFps();
    void release();
};




#endif //MYPLAYER_STREAMVIDEO_H
