//
// Created by sg on 2024/1/11.
//

#ifndef MYPLAYER_STREAMAUDIO_H
#define MYPLAYER_STREAMAUDIO_H


#include "Stream.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C" {
#include "libswresample/swresample.h"
};

class StreamAudio : public Stream {
    friend void playerBufferQueueCallback(SLAndroidSimpleBufferQueueItf queue, void *thiz);
public:
    StreamAudio(JavaCaller *javaCaller, AVFormatContext *avFormatContext,
                AVCodecContext *avCodecContext, AVStream *avStream);

    virtual ~StreamAudio();

    virtual void actualPlay();

    virtual void actualStop();

    int fillBuffer();

    void handeDecodeSpeed();
private:
    SwrContext *swrContext;

    int channelLayout = 0;
    int sampleRate = 0;
    int sampleSize = 0;

    int maxSampleBufferSize = 0;
    uint8_t *buffer = 0;
};


#endif //MYPLAYER_STREAMAUDIO_H
