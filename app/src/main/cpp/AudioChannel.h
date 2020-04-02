//
// Created by poe on 2020/3/24.
//

#ifndef PPLAYER_AUDIOCHANNEL_H
#define PPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <android/native_window.h>
#include <pthread.h>
#include "JavaCallHelper.h"

class AudioChannel : public BaseChannel {

public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext);
    /**
     * 播放音频或视频.
     */
    virtual void play();
    /**
     * 停止播放音频或视频.
     */
    virtual void stop();

    void initOpenSL();

    void decoder();

    int getPcm();

private:
    pthread_t pid_audio_play;
    pthread_t pid_audio_decode;
    SwrContext* swrContext = NULL;
    int out_channels;
    int out_samplesize;
    int out_sample_rate;
public:
    uint8_t * buffer;
};


#endif //PPLAYER_AUDIOCHANNEL_H
