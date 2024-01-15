//
// Created by sg on 2024/1/11.
//

#include "StreamAudio.h"

extern "C" {
#include "libavutil/time.h"
}


void funAudioHandeDecodeSpeed(Stream *thiz) {
    ((StreamAudio *) thiz)->handeDecodeSpeed();
}


StreamAudio::StreamAudio(JavaCaller *javaCaller, AVFormatContext *avFormatContext,
                         AVCodecContext *avCodecContext, AVStream *avStream)
        : Stream(javaCaller, avFormatContext, avCodecContext, avStream) {

    // 通道布局 （左+右 = 2）
    channelLayout = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

    // 采样率 如：44100
    sampleRate = 44100;

    // 每个样本占多少位 如：AV_SAMPLE_FMT_S16占16位
    sampleSize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

    LOGE("channelLayout=%d  sampleRate=%d  sampleSize=%d",channelLayout,sampleRate,sampleSize);

    swrContext = swr_alloc_set_opts(0,
            // 输出
            AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
            // 输入
             channelLayout, avCodecContext->sample_fmt, sampleRate,
            0, 0);
    swr_init(swrContext);

    maxSampleBufferSize = sampleRate * sampleSize * channelLayout;
    buffer = static_cast<uint8_t *>(malloc(maxSampleBufferSize));
    memset(buffer,0,maxSampleBufferSize);


    decodeSpeedHandler = funAudioHandeDecodeSpeed;

}
void StreamAudio::handeDecodeSpeed() {
    while (playQueue.size() > 100 && isPlaying) {
        av_usleep(1000 * 10);
    }
}
void playerBufferQueueCallback(SLAndroidSimpleBufferQueueItf queue, void *thiz) {
    StreamAudio *streamAudio = (StreamAudio *) thiz;
    int dataSize = streamAudio->fillBuffer();

    if (dataSize > 0) {
        (*queue)->Enqueue(queue, streamAudio->buffer, dataSize);
    }
}

int StreamAudio::fillBuffer() {
    int dataSize = 0;
    int ret = 0;
    AVFrame *frame = 0;
    while (isPlaying) {
        ret = playQueue.get(frame);
        if (!isPlaying) break;
        if (ret != 1) continue;

        memset(buffer,0,maxSampleBufferSize);
        int nb = swr_convert(swrContext, &buffer, maxSampleBufferSize,
                             (const uint8_t **)frame->data, frame->nb_samples);

        dataSize = nb * sampleSize * channelLayout;
        break;
    }

    releaseAVFrame(frame);
    return dataSize;
}

StreamAudio::~StreamAudio() {
    if (swrContext) {
        swr_free(&swrContext);
        swrContext = 0;
    }

    if (buffer) {
        free(buffer);
        buffer = 0;
    }
}


void StreamAudio::actualPlay() {
    LOGE("StreamAudio::actualPlay()");

    // engineObj
    SLObjectItf engine;
    slCreateEngine(&engine,0,0,0,0,0);
    (*engine)->Realize(engine,SL_BOOLEAN_FALSE);

    // engineItf
    SLEngineItf engineIterface;
    (*engine)->GetInterface(engine,SL_IID_ENGINE,&engineIterface);


    // SLDataSource
    SLDataLocator_AndroidBufferQueue slDataLocatorBufferQueue =
            {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM slDataFormatPcm = {SL_DATAFORMAT_PCM, 2,
                                        SL_SAMPLINGRATE_44_1,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource slDataSource = {&slDataLocatorBufferQueue,&slDataFormatPcm};


    // SLDataSink
    SLObjectItf slObjectOutputMix;
    (*engineIterface)->CreateOutputMix(engineIterface,&slObjectOutputMix,0,0,0);
    (*slObjectOutputMix)->Realize(slObjectOutputMix,SL_BOOLEAN_FALSE);


    SLDataLocator_OutputMix slDataLocatorOutputMix =
            {SL_DATALOCATOR_OUTPUTMIX,slObjectOutputMix};
    SLDataSink slDataSink = {&slDataLocatorOutputMix,NULL};


    // 用到的功能列表
    SLInterfaceID iterfaceIds[1] = {SL_IID_BUFFERQUEUE};
    SLboolean interfaceRequired[1] = {SL_BOOLEAN_TRUE};
    int interfaceCount = 1;


    //
    SLObjectItf slPlayerObject;
    (*engineIterface)->CreateAudioPlayer(engineIterface,&slPlayerObject,
                                         &slDataSource,
                                         &slDataSink,
                                         interfaceCount,iterfaceIds,interfaceRequired);
    (*slPlayerObject)->Realize(slPlayerObject,SL_BOOLEAN_FALSE);


    SLAndroidSimpleBufferQueueItf slAndroidSimpleBufferQueueItf;
    (*slPlayerObject)->GetInterface(slPlayerObject,SL_IID_BUFFERQUEUE,&slAndroidSimpleBufferQueueItf);
    (*slAndroidSimpleBufferQueueItf)->RegisterCallback(slAndroidSimpleBufferQueueItf,playerBufferQueueCallback,
                                                       this);

    SLPlayItf slPlayItf;
    (*slPlayerObject)->GetInterface(slPlayerObject,SL_IID_PLAY,&slPlayItf);
    (*slPlayItf)->SetPlayState(slPlayItf,SL_PLAYSTATE_PLAYING);


    playerBufferQueueCallback(slAndroidSimpleBufferQueueItf, this);
}

void StreamAudio::actualStop() {
    LOGE("StreamAudio::actualStop()");
}



