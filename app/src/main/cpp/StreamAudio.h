//
// Created by sg on 2024/1/11.
//

#ifndef MYPLAYER_STREAMAUDIO_H
#define MYPLAYER_STREAMAUDIO_H


#include "Stream.h"

class StreamAudio : public Stream{
public:
    StreamAudio(JavaCaller *javaCaller, AVFormatContext* avFormatContext,
                AVCodecContext *avCodecContext, AVStream *avStream);


    virtual void actualPlay();
    virtual void actualStop();


};


#endif //MYPLAYER_STREAMAUDIO_H
