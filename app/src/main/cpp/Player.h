//
// Created by sg on 2024/1/10.
//

#ifndef MYPLAYER_PLAYER_H
#define MYPLAYER_PLAYER_H


#include "JavaCaller.h"
#include "ConcurrentBlockingDeque.h"
#include "StreamVideo.h"
#include "StreamAudio.h"


extern "C" {
#include "libavformat/avformat.h"
};

class Player {
    friend void *prepareRunnable(void *_player);

    friend void *demuxRunnable(void *_player);

public:

    Player(JavaCaller *pCaller);

    virtual ~Player();

    void setPath(const char *_path);

    void prepare();

    void play();

    void release();

    void setWindow(ANativeWindow *pWindow);

private:
    AVFormatContext *avFormatContext = 0;

    char *path = 0;
    JavaCaller *javaCaller;

    pthread_t prepareThread;    // 准备线程，打开媒体文件，探测文件音视频流，打开解码器等
    pthread_t demuxThread;      // 解封装线程

    StreamVideo *streamVideo = 0;
    StreamAudio *streamAudio = 0;

    bool isPlaying = false;

    ANativeWindow *window;
private:
    void prepareInner();

    void demuxInner();      // 媒体文件解封装

    void logErrorAndRelease(int code);

    bool isFinish();
};


#endif //MYPLAYER_PLAYER_H
