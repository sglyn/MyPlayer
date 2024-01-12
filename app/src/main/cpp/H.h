//
// Created by sg on 2024/1/10.
//

#ifndef MYPLAYER_H_H
#define MYPLAYER_H_H

enum Thread {
    THREAD_MAIN,
    THREAD_CHILD
};


//错误代码
enum ErrorCode {
    FFMPEG_CAN_NOT_OPEN_URL,                //打不开视频
    FFMPEG_CAN_NOT_FIND_STREAMS,            //找不到流媒体
    FFMPEG_FIND_DECODER_FAIL,               //找不到解码器
    FFMPEG_ALLOC_CODEC_CONTEXT_FAIL,        //无法根据解码器创建上下文
    FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL,   //根据流信息 配置上下文参数失败
    FFMPEG_OPEN_DECODER_FAIL,               //打开解码器失败
    FFMPEG_NOMEDIA,                         //没有音视频
};


#endif //MYPLAYER_H_H
