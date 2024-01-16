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
    friend void funAudioHandeDecodeSpeed(Stream *thiz);
public:
    StreamAudio(JavaCaller *javaCaller, AVFormatContext *avFormatContext,
                AVCodecContext *avCodecContext, AVStream *avStream);

    virtual ~StreamAudio();

    virtual void actualPlay();

    virtual void actualStop();


private:
    SwrContext *swrContext;

    int channelLayout = 0;
    int sampleRate = 0;
    int sampleSize = 0;

    int maxSampleBufferSize = 0;
    uint8_t *buffer = 0;

    //OpenSLES
    SLObjectItf engine = 0;
    SLEngineItf engineIterface = 0;

    SLObjectItf slObjectOutputMix = 0;
    SLObjectItf slPlayerObject = 0;

    SLAndroidSimpleBufferQueueItf slAndroidSimpleBufferQueueItf = 0;
    SLPlayItf slPlayItf = 0;

private:
    int fillBuffer();
    void handeDecodeSpeed();
    void release();
    void releaseOpenSLES();
};


#endif //MYPLAYER_STREAMAUDIO_H
