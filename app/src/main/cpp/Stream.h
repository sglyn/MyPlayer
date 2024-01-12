//
// Created by sg on 2024/1/11.
//

#ifndef MYPLAYER_STREAM_H
#define MYPLAYER_STREAM_H


#include "JavaCaller.h"
#include "ConcurrentBlockingDeque.h"


extern "C" {
#include "include/libavcodec/avcodec.h"
#include "include/libavformat/avformat.h"
#include "include/libavutil/frame.h"
}

void releaseAVPacket(AVPacket *&packet);

void releaseAVFrame(AVFrame *&frame);


class Stream {
    friend void *decodeRunnable(void *_player);

    typedef void (*DecodeSpeedHandler)(Stream *);

public:


    Stream(JavaCaller *_javaCaller,
           AVFormatContext* avFormatContext,
           AVCodecContext *_avCodecContext,
           AVStream *_avStream);

    virtual ~Stream();

    // 在线程里执行
    virtual void actualDecode();

    // 在线程里执行
    virtual void actualPlay() = 0;

    virtual void actualStop() = 0;


    // 启动流的播放 = 设置标志位 + 开启decode线程 + 开启play线程
    virtual void start() final;

    // 停止播放
    virtual void stop() final;


    int steamIndex();

    // 【解码队列】 和 【播放队列】  里的数据是否已经读取完成
    bool eof();

    void setEnable(bool enable);

    ConcurrentBlockingDeque<AVPacket *> decodeQueue;
    ConcurrentBlockingDeque<AVFrame *> playQueue;

    DecodeSpeedHandler decodeSpeedHandler = 0;
protected:
    bool isPlaying = false;
    JavaCaller *javaCaller = 0;

    AVFormatContext* avFormatContext = 0;
    AVCodecContext *avCodecContext = 0;
    AVStream *avStream = 0;

    pthread_t decodeThread;
    pthread_t playThread;

    AVRational timeBase;

};


#endif //MYPLAYER_STREAM_H
