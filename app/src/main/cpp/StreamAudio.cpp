//
// Created by sg on 2024/1/11.
//

#include "StreamAudio.h"

StreamAudio::StreamAudio(JavaCaller *javaCaller, AVFormatContext* avFormatContext,
                         AVCodecContext *avCodecContext, AVStream *avStream)
        : Stream(javaCaller,avFormatContext, avCodecContext, avStream) {

}

void StreamAudio::actualPlay() {
    LOGE("StreamAudio::actualPlay()");
}

void StreamAudio::actualStop() {
    LOGE("StreamAudio::actualStop()");
}
