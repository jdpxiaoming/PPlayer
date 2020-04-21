//
// Created by poe on 2020/3/24.
//


#include "VideoChannel.h"



/**
 * 丢AVPacket ，丢非I帧.
 * @param q
 */
void dropPacket(queue<AVPacket *> &q){
    LOGE("丢弃视频Packet.....");

    while (!q.empty()){

        AVPacket* pkt = q.front();
        if(pkt->flags != AV_PKT_FLAG_KEY){
            q.pop();
            BaseChannel::releaseAvPacket(pkt);
        }else{
            break;
        }
    }
}

/**
 * 丢掉frame帧. 清空frame队列.
 * @param q
 */
void dropFrame(queue<AVFrame *> &q){
    LOGE("丢弃视频Frame.....");
    while (!q.empty()){
        AVFrame* frame = q.front();
        q.pop();
        BaseChannel::releaseAvFrame(frame);
    }
}

VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext,AVRational time_base,AVFormatContext* formatContext)
        : BaseChannel(id, javaCallHelper, avCodecContext,time_base)
        {
            this->javaCallHelper = javaCallHelper;
            this->avCodecContext = avCodecContext;
            this->avFormatContext = formatContext;
            pkt_queue.setReleaseCallback(releaseAvPacket);
            pkt_queue.setSyncHandle(dropPacket);
            frame_queue.setReleaseCallback(releaseAvFrame);
            frame_queue.setSyncHandle(dropFrame);

        }



/**
 * 解码线程.
 * @param args
 * @return
 */
void * decode(void* args){
    VideoChannel* videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decodePacket();
    return 0;
}


/**
 * 播放线程.
 * @param args
 * @return
 */
void * synchronize(void* args){
    VideoChannel* videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->synchronizeFrame();
    return 0;
}

/**
 * 开启视频解码packet线程 + 视频frame解码渲染线程.
 */
void VideoChannel::play() {
    pkt_queue.setWork(1);
    frame_queue.setWork(1);
    isPlaying = true ;
    //创建一个线程 。
    //解码线程packet->frame.
    pthread_create(&pid_video_play, NULL ,decode , this);
    //播放线程 frame->yuv.
    pthread_create(&pid_synchronize, NULL, synchronize,this);
}

void VideoChannel::stop() {
    //1. set the playing flag false.
    isPlaying = false;
    //2. release thread deque packet thread .
    pthread_join(pid_video_play,NULL);
    //3. release the synchronize thread for frame transform and render .
    pthread_join(pid_synchronize,NULL);
    //4. clear the queue .
    pkt_queue.clear();
    frame_queue.clear();
}

/**
 * 解码出packet队列数据 .
 */
void VideoChannel::decodePacket() {
    AVPacket* packet = 0;
    while (isPlaying){
        //流 --packet --音频 可以 单一  。
        int ret  = pkt_queue.deQueue(packet);
        if(!isPlaying) break;
        if(!ret){
            continue;
        }
//        LOGE("pkt_queue get packet susuccess :%d",ret);
        //解压frame.
//        LOGE("avcodec_send_packet start !");

        if(!avCodecContext){
            LOGE("avCodecContext is NULL!");
        }
        ret = avcodec_send_packet(avCodecContext, packet);
//        LOGE("avcodec_send_packet finished :%d",ret);
//        releaseAvPacket(packet);//释放packet.
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
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        //延缓队列缓存的速度，大于100帧等待10ms。
        while(isPlaying && frame_queue.size() > 100){
            av_usleep(1000*10);
            LOGE("frame queue is full！frame_queue size: %d",frame_queue.size());
            continue;
        }
        //压缩数据要 解压 yuv->rgb888
        //放入缓存队列.
        frame_queue.enQueue(avFrame);
    }

    //保险起见
    releaseAvPacket(packet);
}

void VideoChannel::synchronizeFrame() {

    //从视频流中读取数据包 .
    SwsContext* swsContext = sws_getContext(
            avCodecContext->width,avCodecContext->height,
            avCodecContext->pix_fmt ,
            avCodecContext->width,avCodecContext->height,
            AV_PIX_FMT_RGBA,SWS_BILINEAR,
            0,0,0
    );

    //rgba接收的容器
    uint8_t * dst_data[4];//argb .
    //每一行的首地址
    int dst_linesize[4];

    av_image_alloc(dst_data, dst_linesize , avCodecContext->width ,avCodecContext->height,
                   AV_PIX_FMT_RGBA,1);
    //绘制界面 .
    //转化：YUV->RGB.
    AVFrame* frame = 0;
    while (isPlaying){
        int ret = frame_queue.deQueue(frame);

        if(!isPlaying){
            break;
        }

        if(!ret){
            continue;
        }

        LOGE("synchronizeFrame！get frame success : %d",frame_queue.size());

        sws_scale(swsContext , frame->data, frame->linesize ,0,frame->height,
                  dst_data, dst_linesize);

        frame->pts;
        //已经获取了rgb数据，则回调给native-lib层使用.
        renderFrame (dst_data[0],dst_linesize[0] , avCodecContext->width,avCodecContext->height);
        //暂时没有来做到音视频同步，所以渲染一帧，等待16ms.
        LOGE("解码一帧视频  %d",frame_queue.size());

        clock = frame->pts*av_q2d(time_base);
        //解码一帧视频延时时间.
        double frame_delay = 1.0/fps;
        //解码一帧花费的时间. 配置差的手机 解码耗时教旧，所以需要考虑解码时间.
        double extra_delay = frame->repeat_pict/(2*fps);
        double delay = frame_delay + extra_delay;


        double audioClock = audioChannel->clock;
        double diff = clock - audioClock;

        LOGE(" audio clock %d",audioClock);
        LOGE(" video clock %d",clock);
        LOGE(" fps %d",fps);
        LOGE(" frame_delay %d",frame_delay);
        LOGE(" extra_delay %d",extra_delay);


        LOGE("-----------相差----------  %d ",diff);
        if(clock > audioClock){//视频超前，睡一会.
            LOGE("-----------视频超前，相差----------  %d",diff);
            if(diff>1){
                LOGE("-----------睡眠long----------  %d",(delay*2));
                //差的太久了，那只能慢慢赶 不然卡好久
                av_usleep((delay*2)*1000000);
            }else{
                LOGE("-----------睡眠normal----------  %d",(delay+diff));
                av_usleep((delay+diff)*1000000);
            }

        }else{//音频超前，需要丢帧进行处理 .
            LOGE("-----------音频超前，相差----------  %d",diff);
            if(abs(diff)>1){
                //不休眠.
            }else if(abs(diff) > 0.05){
                //视频需要追赶.丢帧(非关键帧) 同步
                releaseAvFrame(frame);
                frame_queue.sync();
            }else{
                av_usleep((delay+diff)*1000000);
            }
        }

        //释放不需要的frame,frame已经没有利用价值了.
        releaseAvFrame(frame);
    }

    //释放资源.
    av_free(&dst_data[0]);
    isPlaying = false;
    releaseAvFrame(frame);
    sws_freeContext(swsContext);
}

void VideoChannel::setReanderFrame(RenderFrame renderFrame) {
    this->renderFrame = renderFrame;
}

void VideoChannel::setFps(int fps) {
    this->fps = fps;
}


void VideoChannel::seek(long ms) {
    LOGE("VideoChannel::seek has not implemeted!");
}
