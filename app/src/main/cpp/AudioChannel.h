//
// Created by poe on 2020/3/24.
//

#ifndef PPLAYER_AUDIOCHANNEL_H
#define PPLAYER_AUDIOCHANNEL_H

#include "BaseChannel.h"
#include <android/native_window.h>
#include <pthread.h>
#include "JavaCallHelper.h"
#include <SLES/OpenSLES_Android.h>

class AudioChannel : public BaseChannel {

public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *codecContext,AVRational time_base,AVFormatContext* formatContext);
    /**
     * 播放音频或视频.
     */
    virtual void play();
    /**
     * 停止播放音频或视频.
     */
    virtual void stop();

    virtual void seek(long ms);

    void initOpenSL();

    void decoder();

    int getPcm();



private:
    pthread_t pid_audio_play;
    pthread_t pid_audio_decode;
    SwrContext* swrContext = NULL;
    int out_channels; //通道数
    int out_samplesize;//采样率
    int out_sample_rate;//采样频率.
public:
    uint8_t * buffer;
    double  clock;// 音视频同步使用.
};


#endif //PPLAYER_AUDIOCHANNEL_H
