//
// Created by sg on 2024/1/11.
//

#include "Stream.h"
#include "Player.h"


Stream::Stream(JavaCaller *_javaCaller, AVFormatContext* _avFormatContext,
               AVCodecContext *_avCodecContext, AVStream *_avStream)
        : javaCaller(_javaCaller),
        avFormatContext(_avFormatContext),
        avCodecContext(_avCodecContext),
        avStream(_avStream) {

    timeBase = avStream->time_base;

    decodeQueue.setReleaseHandler(releaseAVPacket);
    playQueue.setReleaseHandler(releaseAVFrame);
}

Stream::~Stream() {
    if (avCodecContext) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = 0;
    }

    decodeQueue.clear();
    playQueue.clear();
}

void* decodeRunnable(void* _stream){
    Stream* stream = (Stream*) _stream;
    stream->actualDecode();
    return 0;
}
void* playRunnable(void* _stream){
    Stream* stream = (Stream*) _stream;
    stream->actualPlay();
    return 0;
}

void Stream::start() {
    isPlaying = true;
    setEnable(true);

    pthread_create(&decodeThread,0,decodeRunnable, this);
    pthread_create(&playThread,0,playRunnable, this);
}


int Stream::steamIndex() {
    if (avStream) {
        return avStream->index;
    }
    return -1;
}

bool Stream::eof() {
    bool decodeEmpty = decodeQueue.empty();
    bool playEmpty = playQueue.empty();
    LOGE("decodeSize=%d playSize=%d", decodeQueue.size(), playQueue.size());

    if (decodeEmpty && playEmpty) {
        return true;
    }
    return false;
}

void Stream::setEnable(bool enable) {
    decodeQueue.setEnable(enable);
    playQueue.setEnable(enable);
}

void Stream::stop() {
    isPlaying = false;
    setEnable(false);
    javaCaller = 0;

    pthread_join(decodeThread,0);
    pthread_join(playThread,0);

    actualStop();

}

void Stream::actualDecode() {
    AVPacket *packet = 0;
    while (isPlaying) {
        int ret = decodeQueue.get(packet);
        if (!isPlaying) break;
        if (ret != 1) continue;

        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAVPacket(packet);
        if (ret < 0) { // 不成功
            break;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);

        if (ret == AVERROR(EAGAIN)) {
            LOGE("StreamVideo::actualDecode()  avcodec_receive_frame EAGAIN");
            continue;
        } else if (ret < 0) {
            LOGE("StreamVideo::actualDecode()  avcodec_receive_frame ret = %d",ret , av_err2str(ret));
            break;
        }
        // 控制解码的速度
        if(decodeSpeedHandler){
            decodeSpeedHandler(this);
        }
        playQueue.put(frame);
    }
    releaseAVPacket(packet);
}

void releaseAVPacket(AVPacket *&packet) {
    if (packet) {
        av_packet_free(&packet);
        packet = 0;
    }
}

void releaseAVFrame(AVFrame *&frame) {
    if (frame) {
        av_frame_free(&frame);
        frame = 0;
    }
}
