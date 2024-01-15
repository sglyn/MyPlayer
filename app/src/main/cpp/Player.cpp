//
// Created by sg on 2024/1/10.
//

#include <cstring>
#include "Player.h"

extern "C"{
#include <libavutil/time.h>
}

Player::Player(JavaCaller *pCaller) : javaCaller(pCaller) {


}

void Player::setPath(const char *p) {
    if (path) {
        delete[] path;
    }
    path = new char[strlen(p) + 1];
    strcpy(path, p);
    LOGE("Player::setPath()  path=%s", path);
}

Player::~Player() {
    avformat_network_deinit();
    if (path) {
        delete[] path;
        path = 0;
    }

    if (javaCaller) {
        delete javaCaller;
        javaCaller = 0;
    }
}

void *prepareRunnable(void *_player) {
    Player *player = (Player *) _player;
    player->prepareInner();
    return 0;
}

void Player::prepare() {
    LOGE("Player::prepare()");
    pthread_create(&prepareThread, 0, prepareRunnable, this);
}

// 此方法在子线程里执行
void Player::prepareInner() {
    LOGE("Player::prepareInner start");

    // 初始化网络
    avformat_network_init();

    // 初始化 “解封装” 上下文AVFormatContext
    avFormatContext = avformat_alloc_context();

    // 打开媒体文件，第三个参数传入0，表示自动测试文件格式。 第四个为打开媒体文件的参数，如打开网络媒体文件超时等
    avformat_open_input(&avFormatContext, path, 0, 0);
    avformat_find_stream_info(avFormatContext, 0);  // 自动探测媒体文件

    int nbStreams = avFormatContext->nb_streams;    // 媒休文件有多少路流（视频流 + 音频流等）
    for (int i = 0; i < nbStreams; i++) {
        AVStream *avStream = avFormatContext->streams[i];

        AVCodecParameters *aVCodecParameters = avStream->codecpar;
        AVCodecID avCodecId = aVCodecParameters->codec_id;
        // AVCodecID=27  name=h264
        // AVCodecID=86018  name=aac
        LOGE("AVCodecID=%d  name=%s", avCodecId, avcodec_get_name(avCodecId));


        AVCodec *avCodec = avcodec_find_decoder(avCodecId);                     // 查找avCodecId对应的解码器
        AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);   // 给解码器分配上下文
        avcodec_parameters_to_context(avCodecContext,
                                      aVCodecParameters); // 把aVCodecParameters里的数据copy到avCodecContext
        AVMediaType mediaType = avCodecContext->codec_type;
        // meidaType=0  name=video
        // meidaType=1  name=audio
        LOGE("meidaType=%d  name=%s", mediaType, av_get_media_type_string(mediaType));

        avcodec_open2(avCodecContext, avCodec, 0);// 打开解码器

        LOGE("i=%d  avStream->index=%d", i, avStream->index);

        if (mediaType == AVMEDIA_TYPE_VIDEO) {
            streamVideo = new StreamVideo(javaCaller, avFormatContext,avCodecContext, avStream);
            streamVideo->setWindow(window);
        } else if (mediaType == AVMEDIA_TYPE_AUDIO) {
           // streamAudio = new StreamAudio(javaCaller,avFormatContext, avCodecContext, avStream);
        }
    }

    if (!streamVideo && !streamAudio) {
        javaCaller->onError(FFMPEG_NOMEDIA, THREAD_CHILD);
    }

    javaCaller->onPrepared(THREAD_CHILD);
    LOGE("Player::prepareInner() end");
}


void *demuxRunnable(void *_player) {
    Player *player = (Player *) _player;
    player->demuxInner();
    return 0;
}

// 开始播放。 1.解封装，把媒体文件每帧读取到
void Player::play() {
    LOGE("Player::play() start");

    isPlaying = true;
    if (streamVideo) {
        streamVideo->start();
    }

    if (streamAudio) {
        streamAudio->start();
    }
    pthread_create(&demuxThread, 0, demuxRunnable, this);
}

// 此方法在子线程里执行
void Player::demuxInner() {
    while (isPlaying) {
        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(avFormatContext, packet);
        // 读取成功
        if (ret == 0) {
            if (streamVideo && packet->stream_index == streamVideo->steamIndex()) {
                streamVideo->decodeQueue.put(packet);
            } else if (streamAudio && packet->stream_index == streamAudio->steamIndex()) {
                //streamAudio->decodeQueue.put(packet);
                av_packet_free(&packet);
            } else {
                av_packet_free(&packet);
            }
        } else {
            av_packet_free(&packet);
            if (ret == AVERROR_EOF) {
                if (isFinish()) {
                    LOGE("Video.decodeQueue.size=%d",streamVideo->decodeQueue.size());
                    //LOGE("Audio.playQueue.size=%d",streamAudio->playQueue.size());
                    break;
                }
            } else {
                LOGE("读取数据失败 code=%d 描述：%s", ret, av_err2str(ret));
                break;
            }
        }
    }

    isPlaying = false;

    if(streamVideo){
        streamVideo->stop();
    }

    if(streamAudio){
        streamAudio->stop();
    }

}

// 音频流+视频流 是否播放完成   播放完成=该音/视频流的decode队列和play队列都为空
bool Player::isFinish() {
    bool eofVideo = true;
    if (streamVideo && !streamVideo->eof()) {
        eofVideo = false;
    }
    bool eofAudio = true;
    if (streamAudio && !streamAudio->eof()) {
        eofAudio = false;
    }
    //LOGE("eof=%d, eofAudio=%d  eofVideo=%d", eofAudio && eofVideo, eofAudio, eofVideo);
    return eofAudio && eofVideo;
}

void Player::release() {

    if (avFormatContext) {
        avformat_close_input(&avFormatContext);
        avformat_free_context(avFormatContext);
        avFormatContext = 0;
    }

    if(javaCaller){
        delete javaCaller;
        javaCaller = 0;
    }

    if(streamVideo){
        delete streamVideo;
        streamVideo = 0;
    }
    if(streamAudio){
        delete streamAudio;
        streamAudio = 0;
    }
}

void Player::logErrorAndRelease(int code) {
    //LOGE("打开文件失败 code=%d path=%s  错误描述：", code, path, av_err2str(code));
    //release();
}

void Player::setWindow(ANativeWindow *_window) {
    this->window = _window;
    if(streamVideo){
        streamVideo->window = _window;
    }
}
