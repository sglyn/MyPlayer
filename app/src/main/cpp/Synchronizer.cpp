//
// Created by sg on 2024/1/16.
//

#include "Synchronizer.h"


extern "C" {
#include "libavutil/common.h"
}
double Synchronizer::calcDelay(double videoDalay) {
    double delay = videoDalay;


    /**
         * 1、delay < 0.04, 同步阈值就是0.04
         * 2、delay > 0.1 同步阈值就是0.1
         * 3、0.04 < delay < 0.1 ，同步阈值是delay
         */
    // 根据每秒视频需要播放的图象数，确定音视频的时间差允许范围
    // 给到一个时间差的允许范围
    double sync = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));

    double diff = clockVideo - clockAudio;


    const double eps=1e-6;//1的负6次方

    //视频落后了，落后太多了，我们需要去同步
    if (diff <= -sync) {
        //就要让delay时间减小
        delay = FFMAX(0, delay + diff);
    } else if (diff > sync) {
        //视频快了,让delay久一些，等待音频赶上来
        delay = delay + diff;
    }

    return delay;
}
