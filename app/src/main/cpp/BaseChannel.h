//
// Created by poe on 2020/3/25.
//

#ifndef PPLAYER_BASECHANNEL_H
#define PPLAYER_BASECHANNEL_H

#include <android/log.h>
#include "safe_queue.h"
#include "JavaCallHelper.h"

extern "C"{
#include <libavutil/rational.h>
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
//编码转换工具yuv->rgb888
#include "libavutil/imgutils.h"
}

#define TAG "poe" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

class BaseChannel {
public:
    BaseChannel(int id, JavaCallHelper* javaCallHelper,
            AVCodecContext* codecContext):channelId(id),javaCallHelper(javaCallHelper),avCodecContext(codecContext)
    {

    }
    ~BaseChannel(){
        //销毁解码器上下文.
        if(avCodecContext){
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = 0;
        }
        //销毁队列 ,此处有问题safe_queue.clear()，SafeQueue结构体未明确.
        pkt_queue.clear();
        frame_queue.clear();
        //
        LOGE("释放channel:%d %d" , pkt_queue.size(), frame_queue.size());
    }

    static void releaseAvPacket(AVPacket*& packet){
        if(packet){
            av_packet_free(&packet);
            packet = 0;
        }
    }

    static void releaseAvFrame(AVFrame*& frame){
        if(frame){
            av_frame_free(&frame);
            frame = 0;
        }
    }

    /**
     * 播放音频或视频.
     */
    virtual void play()=0;
    /**
     * 停止播放音频或视频.
     */
    virtual void stop()=0;

public:
    SafeQueue<AVPacket *> pkt_queue;
    SafeQueue<AVFrame *> frame_queue;
    volatile int channelId;
    volatile bool isPlaying;
    AVCodecContext* avCodecContext;
    JavaCallHelper* javaCallHelper;
};


#endif //PPLAYER_BASECHANNEL_H
