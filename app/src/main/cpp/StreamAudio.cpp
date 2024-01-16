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
    buffer = (uint8_t *)malloc(maxSampleBufferSize);
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
    LOGE("StreamAudio::~StreamAudio()");
    release();
}


void StreamAudio::actualPlay() {
    LOGE("StreamAudio::actualPlay()");

    // engineObj
    slCreateEngine(&engine,0,0,0,0,0);
    (*engine)->Realize(engine,SL_BOOLEAN_FALSE);

    // engineItf
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
    // 创建一个混音器对象
    (*engineIterface)->CreateOutputMix(engineIterface,&slObjectOutputMix,0,0,0);
    (*slObjectOutputMix)->Realize(slObjectOutputMix,SL_BOOLEAN_FALSE);


    SLDataLocator_OutputMix slDataLocatorOutputMix =
            {SL_DATALOCATOR_OUTPUTMIX,slObjectOutputMix};
    SLDataSink slDataSink = {&slDataLocatorOutputMix,NULL};


    // 用到的功能接口
    SLInterfaceID iterfaceIds[1] = {SL_IID_BUFFERQUEUE};
    SLboolean interfaceRequired[1] = {SL_BOOLEAN_TRUE};
    // 用到功能接口的个数
    int interfaceCount = 1;


    //slPlayerObject
    (*engineIterface)->CreateAudioPlayer(engineIterface,&slPlayerObject,
                                         &slDataSource,
                                         &slDataSink,
                                         interfaceCount,iterfaceIds,interfaceRequired);
    (*slPlayerObject)->Realize(slPlayerObject,SL_BOOLEAN_FALSE);

    // SLAndroidSimpleBufferQueueItf
    (*slPlayerObject)->GetInterface(slPlayerObject,SL_IID_BUFFERQUEUE,&slAndroidSimpleBufferQueueItf);
    (*slAndroidSimpleBufferQueueItf)->RegisterCallback(slAndroidSimpleBufferQueueItf,playerBufferQueueCallback,
                                                       this);
    // SLPlayItf
    (*slPlayerObject)->GetInterface(slPlayerObject,SL_IID_PLAY,&slPlayItf);
    (*slPlayItf)->SetPlayState(slPlayItf,SL_PLAYSTATE_PLAYING);

    // 手动调用一次，启动播放
    playerBufferQueueCallback(slAndroidSimpleBufferQueueItf, this);
}

void StreamAudio::actualStop() {
    LOGE("StreamAudio::actualStop()");
}

void StreamAudio::release() {
    if (swrContext) {
        swr_free(&swrContext);
        swrContext = 0;
    }

    if (buffer) {
        free(buffer);
        buffer = 0;
    }

    releaseOpenSLES();
}

void StreamAudio::releaseOpenSLES() {
    if(slPlayItf){
        (*slPlayItf)->SetPlayState(slPlayItf,SL_PLAYSTATE_STOPPED);
        slPlayItf = 0;
    }

    if(slAndroidSimpleBufferQueueItf){
        (*slAndroidSimpleBufferQueueItf)->Clear(slAndroidSimpleBufferQueueItf);
        slAndroidSimpleBufferQueueItf = 0;
    }

    if(slObjectOutputMix){
        (*slObjectOutputMix)->Destroy(slObjectOutputMix);
        slObjectOutputMix = 0;
    }

    if(slPlayerObject){
        (*slPlayerObject)->Destroy(slPlayerObject);
        slPlayerObject = 0;
    }

    if(engine){
        (*engine)->Destroy(engine);
        engine = 0;
        engineIterface = 0;
    }

}



