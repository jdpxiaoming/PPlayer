//
// Created by poe on 2020/3/24.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,AVRational time_base, AVFormatContext* formatContext)
        : BaseChannel(id, javaCallHelper, avCodecContext,time_base)
{
    this->javaCallHelper = javaCallHelper;
    this->avCodecContext = avCodecContext;
    this->avFormatContext = formatContext;
    //初始化音频相关参数
    out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    out_samplesize = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    //CD音频标准
    //44100 双声道 2字节
    buffer = (uint8_t *)(malloc(out_sample_rate * out_samplesize * out_channels));
    //设置清空回调函数释放对象的回调.
    pkt_queue.setReleaseCallback(releaseAvPacket);
    frame_queue.setReleaseCallback(releaseAvFrame);
}
// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{

    LOGE("bqPlayerCallback()#invoked!");
    AudioChannel* audioChannel = static_cast<AudioChannel *>(context);
    int datalen = audioChannel->getPcm();
    LOGE("bqPlayerCallback()# datalen:%d",datalen);

    if(datalen >0 ){
        (*bq)->Enqueue(bq,audioChannel->buffer,datalen);
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
//    初始化转换器上下文,设置重采样 .
    swrContext = swr_alloc_set_opts(0,
                        AV_CH_LAYOUT_STEREO,
                        AV_SAMPLE_FMT_S16,
                        out_sample_rate,
                        avCodecContext->channel_layout,
                        avCodecContext->sample_fmt,
                        avCodecContext->sample_rate,
                        0,
                        0);
    //初始化转换器的其他参数.
    swr_init(swrContext);
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
    //1. set the playing flag false.
    isPlaying = false;
    //2. release thread deque packet thread .
    pthread_join(pid_audio_decode,NULL);
    //3. release the synchronize thread for frame transform and render .
    pthread_join(pid_audio_play,NULL);
    //4. clear the queue .
    pkt_queue.clear();
    frame_queue.clear();
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
    // 回调接口.
    SLPlayItf  bqPlayerInterface = NULL;
    //4. 创建缓冲队列和回调函数
//    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = NULL;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue ;
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
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if(SL_RESULT_SUCCESS != result){
        return ;
    }

    // ---------------------------2. 初始化混音器----------------------------------------------------
    //创建混音器.
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 1, ids, req);
    //初始化混音器.
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
        return ;
    }

    //5. 设置播放状态
    //创建播放器.双声道最后写2. configure audio source
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};

    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM,//播放pcm格式数据.
                                   2,                //2个声道(立体声).
                                   SL_SAMPLINGRATE_44_1,//44100hz的频率.
                                   SL_PCMSAMPLEFORMAT_FIXED_16,//位数16bit
                                   SL_PCMSAMPLEFORMAT_FIXED_16,//位数16bit.
                                   SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,//立体声(前左前右)
                                   SL_BYTEORDER_LITTLEENDIAN//小端模式.
    };

    SLDataSource audioSrc = {&android_queue, &pcm};

    // configure audio sink
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};
    /*
        * create audio player:
        *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
        *     for fast audio case
        */
    const SLInterfaceID ids2[1] = {SL_IID_BUFFERQUEUE/*, SL_IID_VOLUME, SL_IID_EFFECTSEND,
            SL_IID_MUTESOLO,*/};
    const SLboolean req2[1] = {SL_BOOLEAN_TRUE/*, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            SL_BOOLEAN_TRUE,*/ };
    (*engineInterface)->CreateAudioPlayer(engineInterface
                                    ,&bqPlayerObject //播放器
                                    ,&audioSrc//播放器参数 播放器缓冲队列 播放格式
                                    ,&audioSnk // 播放缓冲区
                                    ,1//播放接口回调个数.
                                    ,ids2 //设置播放队列ID
                                    ,req2 //是否采用内置的播放队列
                                    );
    //初始化播放器.
    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
        return ;
    }
    LOGE("realize the player success!");
    //获取播放器接口player interface.
    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);

    //获取pcm缓冲队列.
    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);
    //注册回调函数.
    if(SL_RESULT_SUCCESS != result){
        return ;
    }
    LOGE("get the buffer queue interface success!");

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
    if(SL_RESULT_SUCCESS != result){
        return ;
    }
    LOGE("RegisterCallback success!");
    //设置播放状态.
    result = (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);
    if(SL_RESULT_SUCCESS != result){
        return ;
    }

    LOGE("SetPlayState SL_PLAYSTATE_PLAYING success!");
    //6. 启动回调函数.
    LOGE("手动调用播放器 packet:%d",this->pkt_queue.size());
    bqPlayerCallback(bqPlayerBufferQueue , this);
}

void AudioChannel::decoder() {
    LOGE("AudioChannel::decoder()  ! isPlaying: %d",isPlaying);
    AVPacket* packet = 0;
    while (isPlaying){
        //音频的pakcket.
        LOGE("AudioChannel::decoder() #dequeue start !");
         int ret = pkt_queue.deQueue(packet);
        LOGE("AudioChannel::decoder() #dequeue success# !pkt_queue.size %d",pkt_queue.size());
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
        LOGE("avcodec_receive_frame success ! avFrame:%s",avFrame);

        if(ret == AVERROR(EAGAIN)){
            //需要更多数据
            continue;
        }else if(ret < 0 ){
            LOGE("avcodec_receive_frame FAilure ret < 0 %d",ret);
            //失败
            break;
        }
        //packet -》fram.
        frame_queue.enQueue(avFrame);
        LOGE("audio frame_queue enQueue success ! :%d",frame_queue.size());
        while (frame_queue.size() > 100 && isPlaying){
            LOGE("frame_queue %d is full, sleep 16 ms",frame_queue.size());
            av_usleep(16*1000);
            continue;
        }
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

        //计算当前音频的播放时钟clock. pts相对数量 time_base:时间单位（1,25）表示1/25分之一秒.
        clock = frame->pts*av_q2d(time_base);
        LOGW("audio clock pts is %d",frame->pts);
        LOGE("audio clock is %f",clock);
        break;
    }

    releaseAvFrame(frame);
    return data_size;
}


//seek frame ..
void AudioChannel::seek(long ms){
    if(ms > 0){
        int64_t timestamp;
        timestamp += ms;
       int ret =  avformat_seek_file(avFormatContext,channelId,INT64_MIN, timestamp , INT64_MAX, AVSEEK_FLAG_BACKWARD);

        if (ret < 0) {
            LOGE("could not seek to position %0.3f\n",(double)timestamp / AV_TIME_BASE);
        }

    }
}
