//
// Created by poe on 2020/3/24.
//

#include "AudioChannel.h"
AudioChannel::AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext)
        : BaseChannel(id, javaCallHelper, avCodecContext)
{}


void AudioChannel::play() {

}

void AudioChannel::stop() {

}