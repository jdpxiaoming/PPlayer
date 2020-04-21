//
// Created by poe on 2020/3/24.
//

#ifndef PPLAYER_POEFFMPEG_H
#define PPLAYER_POEFFMPEG_H

#include <pthread.h>
#include <android/log.h>
#include "android/native_window_jni.h"
#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"

extern "C"{
//封装格式，总上下文
#include "libavformat/avformat.h"
//解码器.
#include "libavcodec/avcodec.h"
//#缩放
#include "libswscale/swscale.h"
// 重采样
#include "libswresample/swresample.h"
//时间工具
#include "libavutil/time.h"
}


#define TAG "poe" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

//播放控制.
class PoeFFmpeg {
public:
    PoeFFmpeg(JavaCallHelper* _javaCallHelper, const char* dataSource);
    ~PoeFFmpeg();

    void prepare();
    void prepareFFmpeg();
    void start();
    void play();
    void pause();
    void close();
    void setRenderCallBack(RenderFrame renderFrame);
    void seek(long ms);
public:
    bool isPlaying;
    pthread_t pid_prepare;//准备完成后销毁.
    pthread_t pid_play;// 解码线程，一直存在知道播放完成.
    char* url;
    AVFormatContext* formatContext;
    JavaCallHelper* javaCallHelper;
    VideoChannel* videoChannel;
    AudioChannel* audioChannel;
    RenderFrame renderFrame;
};




#endif //PPLAYER_POEFFMPEG_H
