//
// Created by poe on 2020/3/24.
//


#include "VideoChannel.h"

VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext)
        : BaseChannel(id, javaCallHelper, avCodecContext)
        {
            this->javaCallHelper = javaCallHelper;
            this->avCodecContext = avCodecContext;
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
        //已经获取了rgb数据，则回调给native-lib层使用.
        renderFrame (dst_data[0],dst_linesize[0] , avCodecContext->width,avCodecContext->height);
        //暂时没有来做到音视频同步，所以渲染一帧，等待16ms.
        av_usleep(1000*16);
        LOGE("解码一帧视频  %d",frame_queue.size());
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
