PPlayer 基于Ffmpeg4.0.2(arm64-v8a)静态库搭建一套高可用的播播放器
===
>ffmpeg 4.0.2静态库从0开始一个播放器的搭建，支持rtmp、rtsp、hls、本地MP4文件播放，视频解码+音频解码+音视频同步

![pic](https://github.com/jdpxiaoming/PPlayer/blob/master/capture/output2.gif)

# 如果有32位播放需求，欢迎给我提`issue`，有时间会把编译ffmpeg的教程也放上来。



# ToDoList
- [x] 视频解码
- [x] 音频解码
- [ ] 音视频同步


# 运行demo
1. 直接使用AndroidStudio导入本项目运行app
2. 拷贝`capture/input.mp4`到`/data/com.poe.pplayer/cache`
3. 点击"播放"

# 本项目默认支持`rtsp`和`rtmp`的播放，只需要在项目中把文件地址换位对应uri（未测试）.



# License
    Copyright 2020 Poe.Cai.

    FFmpeg codebase is mainly LGPL-licensed with optional components licensed under GPL. Please refer to the LICENSE file for detailed information.


