//
// Created by poe on 2020/3/24.
//

#include "AudioChannel.h"

#include <SLES/OpenSLES_Android.h>

AudioChannel::AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext)
        : BaseChannel(id, javaCallHelper, avCodecContext)
{
    this->javaCallHelper = javaCallHelper;
    this->avCodecContext = avCodecContext;
}
// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf caller, void *context)
{

    AudioChannel* audioChannel = static_cast<AudioChannel *>(context);
    int datalen = audioChannel->getPcm();
    LOGE("bqPlayerCallback()# datalen:%d",datalen);

    if(datalen >0 ){
        (*caller)->Enqueue(caller,audioChannel->buffer,datalen);
    }
}

void* audioPlay(void* args){
    AudioChannel* audioChannel = static_cast<AudioChannel*> (args);
    audioChannel->initOpenSL();
    return 0;
}

void* audioDecode(void* args){
    AudioChannel* audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->decoder();

    return 0;
}

void AudioChannel::play() {
//    初始化转换器上下文
    swrContext = swr_alloc_set_opts(0,AV_CH_LAYOUT_STEREO,AV_SAMPLE_FMT_S16,out_sample_rate,
                        avCodecContext->channel_layout,
                        avCodecContext->sample_fmt,
                        avCodecContext->sample_rate,0,0);

    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true ;
    //创建初始化OPENSL_ES的线程

    //创建音频解码线程
    pthread_create(&pid_audio_play, NULL ,audioPlay , this);
    //播放线程 frame->yuv.
    pthread_create(&pid_audio_decode, NULL, audioDecode,this);
}

void AudioChannel::stop() {

}



/**
 * 初始化音频解码.
 */
void AudioChannel::initOpenSL() {
    LOGE("initOpenSL() !");
    //1. 音频引擎
    SLEngineItf engineInterface = NULL;
    //音频对比昂
    SLObjectItf engineObject = NULL;
    //2. 设置混音器
    SLObjectItf outputMixObject = NULL;
    //3. 创建播放器
    SLObjectItf  bqPlayerObject = NULL;
    SLPlayItf  bqPlayerInterface = NULL;
    //4. 创建缓冲队列和回调函数
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;

    //创建音频引擎 .

    // ----------------------------1. 初始化播放器引擎-----------------------------------------------
    SLresult  result =   slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if(SL_RESULT_SUCCESS != result){
        return ;
    }
    // realize the engine .
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
        return ;
    }
    //获取音频接口，相当于surfaceHolder对于surfaceView的控制一样.
    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if(SL_RESULT_SUCCESS != result){
        return ;
    }


    // ---------------------------2. 初始化混音器----------------------------------------------------
    // create output mix, with environmental reverb specified as a non-required interface
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, 0, 0);
    //初始化混音器.
    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
        return ;
    }

    //5. 设置播放状态
    //创建播放器.双声道最后写2.
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM,//播放pcm格式数据.
                                   2,                //2个声道(立体声).
                                   SL_SAMPLINGRATE_44_1,//44100hz的频率.
                                   SL_PCMSAMPLEFORMAT_FIXED_16,//位数16bit
                                   SL_PCMSAMPLEFORMAT_FIXED_16,//位数16bit.
                                   SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,//立体声(前左前右)
                                   SL_BYTEORDER_LITTLEENDIAN//小端模式.
    };

    SLDataSource slDataSource = {&android_queue, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    /*
        * create audio player:
        *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
        *     for fast audio case
        */
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE/*, SL_IID_VOLUME, SL_IID_EFFECTSEND,
            SL_IID_MUTESOLO,*/};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE/*, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE,*/ };
    (*engineInterface)->CreateAudioPlayer(engineInterface
                                    ,&bqPlayerObject //播放器
                                    ,&slDataSource//播放器参数 播放器缓冲队列 播放格式
                                    ,&audioSnk // 播放缓冲区
                                    ,1//播放接口回调个数.
                                    ,ids //设置播放队列ID
                                    ,req //是否采用内置的播放队列
                                    );
    //初始化播放器.
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

    // get the play interface
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);

    // get the buffer queue interface
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,&bqPlayerBufferQueue);
    //注册回调函数.
    // register callback on the buffer queue
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback ,this);
    //设置播放状态.
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface,SL_PLAYSTATE_PLAYING);
    //6. 启动回调函数.
    bqPlayerCallback(bqPlayerBufferQueue , this);
    LOGE("手动调用播放器 packet:%d",this->pkt_queue.size());
}

void AudioChannel::decoder() {
    LOGE("AudioChannel::decoder()  !");
    AVPacket* packet = 0;
    while (isPlaying){
        //音频的pakcket.
         int ret = pkt_queue.deQueue(packet);
         if(!isPlaying){
             break;
         }

         if(!ret){
             continue;
         }
        LOGE("avcodec_send_packet start ! codecContext:%s",avCodecContext);
         //packet送去解码
         ret = avcodec_send_packet(avCodecContext, packet);
         releaseAvPacket(packet);
        if(ret == AVERROR(EAGAIN)){
            LOGE("avcodec_send_packet EAGAIN 等待数据包！");
            //需要更多数据
            continue;
        }else if(ret < 0 ){
            LOGE("avcodec_send_packet FAilure ret < 0 %d",ret);
            //失败
            break;
        }

        AVFrame* avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext , avFrame);
        if(ret == AVERROR(EAGAIN)){
            //需要更多数据
            continue;
        }else if(ret < 0 ){
            LOGE("avcodec_receive_frame FAilure ret < 0 %d",ret);
            //失败
            break;
        }
        //packet -》fram.
        while (frame_queue.size() > 100 && isPlaying){
            av_usleep(1*1000);
            continue;
        }
        frame_queue.enQueue(avFrame);
    }
    releaseAvPacket(packet);
}

/**
 *  获取音频解码的pcm .
 * @return
 */
int AudioChannel::getPcm() {
    LOGE("AudioChannel::getPcm()  %d",frame_queue.size());
    AVFrame* frame = 0;
    int data_size = 0;
    while (isPlaying){
        int ret = frame_queue.deQueue(frame);
        //转换.

        if(!isPlaying){
            break;
        }

        if(!ret){
            continue;
        }


        //frame -> 转化为pcm数据.
        uint64_t  dst_nb_samples = av_rescale_rnd(
                swr_get_delay(swrContext, frame->sample_rate) + frame->nb_samples,
                out_sample_rate,
                frame->sample_rate,
                AV_ROUND_UP
                );
        //转换 返回值为转换后的sample个数.
        int nb = swr_convert(swrContext, &buffer, dst_nb_samples,
                             (const uint8_t**)frame->data, frame->nb_samples);

        //计算转换后buffer的大小 44100*2（采样位数2个字节）*2（双通道）.  。
        data_size = nb * out_channels*out_samplesize;

        break;
    }

    releaseAvFrame(frame);
    return data_size;
}
