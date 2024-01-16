//
// Created by sg on 2024/1/16.
//

#ifndef MYPLAYER_SYNCHRONIZER_H
#define MYPLAYER_SYNCHRONIZER_H


#define AV_SYNC_THRESHOLD_MIN 0.04
#define AV_SYNC_THRESHOLD_MAX 0.1


class Synchronizer {
public:
    double clockVideo = 0.0;
    double clockAudio = 0.0;

    double calcDelay(double videoDalay);

};


#endif //MYPLAYER_SYNCHRONIZER_H
