//
// Created by poe on 2020/3/24.
//

#ifndef PPLAYER_MACRO_H
#define PPLAYER_MACRO_H
#include <android/log.h>
#define TAG "poe" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

#define THREAD_MAIN 1
#define THREAD_CHILD 2

//错误代码
//打不开视频流
#define FFMPEG_CAN_NOT_OPEN_URL 1
//找不到流媒体
#define FFMPEG_CAN_NOT_FIND_STREAMS 2
//找不到解码器
#define FFMPEG_FIND_DECODER_FAIL 3
//无法根据解码器创建上下文
#define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
//根据留信息 配置上下文失败
#define FFMPEG_CODEC_PARAMETERS_FAIL 6
//打开解码器失败
#define FFMPEG_OPEN_DECODER_FIAL 7
//没有音视频
#define FFMPEG_NO_MEDIA 8

#endif //PPLAYER_MACRO_H
