//
// Created by sg on 2024/1/11.
//

#include "StreamVideo.h"


extern "C" {
#include "libavutil/time.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
}

void funHandeVideoDecodeSpeed(Stream *thiz) {
    ((StreamVideo *) thiz)->handeDecodeSpeed();
}


StreamVideo::StreamVideo(JavaCaller *javaCaller, AVFormatContext *avFormatContext,
                         AVCodecContext *avCodecContext, AVStream *avStream)
        : Stream(javaCaller, avFormatContext, avCodecContext, avStream) {

    LOGE("StreamVideo::StreamVideo()..");
    decodeSpeedHandler = funHandeVideoDecodeSpeed;
    calcFps();
    pthread_mutex_init(&surfaceMutex, 0);
}


StreamVideo::~StreamVideo() {
    LOGE("StreamVideo::~StreamVideo()");

    pthread_mutex_destroy(&surfaceMutex);
    release();
}

void StreamVideo::handeDecodeSpeed() {
    while (isPlaying && playQueue.size() > fps * 10) {
        av_usleep(1000 * 10);
    }
}

void StreamVideo::actualPlay() {
    LOGE("StreamVideo::actualPlay()");
    int width = avCodecContext->width;
    int height = avCodecContext->height;

    SwsContext *swsContext = sws_getContext(width, height, avCodecContext->pix_fmt,
                                            width, height, AV_PIX_FMT_RGBA,
                                            SWS_FAST_BILINEAR, 0, 0, 0);

    uint8_t *data[4];
    int linesizes[4];
    av_image_alloc(data, linesizes, width, height, AV_PIX_FMT_RGBA, 1);

    AVFrame *frame = 0;
    double frameDelay = 1.0 / fps;
    double extraDelay = 0;
    double delay = 0;

    int ret = 0;
    while (isPlaying) {

        ret = playQueue.get(frame);

        if (!isPlaying) break;
        if (ret != 1) continue;

        clock = frame->best_effort_timestamp * av_q2d(timeBase);
        synchronizer->clockVideo = clock;

        extraDelay = frame->repeat_pict / (2 * fps);
        delay = syc(extraDelay + frameDelay);

        av_usleep(delay * 1000000);

        sws_scale(swsContext,
                  frame->data, frame->linesize, 0, frame->height,
                  data, linesizes);

        draw(data, linesizes);

        releaseAVFrame(frame);
    }

    isPlaying = false;
    av_freep(&data[0]);
    releaseAVFrame(frame);
    sws_freeContext(swsContext);
}

void StreamVideo::draw(uint8_t **data, int *linesizes) {
    pthread_mutex_lock(&surfaceMutex);
    int width = avCodecContext->width;
    int height = avCodecContext->height;

    if (window) {
        int ret = ANativeWindow_setBuffersGeometry(window,
                                                   width, height,
                                                   WINDOW_FORMAT_RGBA_8888);

        ANativeWindow_Buffer buffer;

        ret = ANativeWindow_lock(window, &buffer, 0);
        // 0： success, negative： error.
        if (ret != 0) {
            ANativeWindow_release(window);
            window = 0;
            pthread_mutex_unlock(&surfaceMutex);
            return;
        }

        uint8_t *destData = static_cast<uint8_t *>(buffer.bits);
        int destSize = buffer.stride * 4;

        uint8_t *srcData = data[0];
        int srcSize = linesizes[0];

        for (int i = 0; i < height; i++) {
            memcpy(destData + i * destSize, srcData + i * srcSize, srcSize);
        }
        ANativeWindow_unlockAndPost(window);
    }

    pthread_mutex_unlock(&surfaceMutex);
}

void StreamVideo::actualStop() {
    LOGE("StreamVideo::actualStop()");
    release();
}


void StreamVideo::calcFps() {
    // 帧率
    fps = av_q2d(avStream->avg_frame_rate);
    if (isnan(fps) || fps == 0) {
        fps = av_q2d(avStream->r_frame_rate);
    }
    if (isnan(fps) || fps == 0) {
        fps = av_q2d(av_guess_frame_rate(avFormatContext, avStream, 0));
    }
    LOGE("fps=%f", fps);
}

ANativeWindow *StreamVideo::getWindow() const {
    return window;
}

void StreamVideo::setWindow(ANativeWindow *window_) {
    pthread_mutex_lock(&surfaceMutex);

    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = window_;

    pthread_mutex_unlock(&surfaceMutex);
}

double StreamVideo::syc(double curVideoFrameDelay) {
    // 音视频同步
    if(synchronizer){
        return synchronizer->calcDelay(curVideoFrameDelay);
    }

    return curVideoFrameDelay;
}

void StreamVideo::release() {
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
}




