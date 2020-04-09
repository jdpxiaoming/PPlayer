PPlayer 基于Ffmpeg4.0.2(arm64-v8a)静态库搭建一套高可用的播放器
===
>ffmpeg 4.0.2静态库从0开始一个播放器的搭建，支持rtmp、rtsp、hls、本地MP4文件播放，视频解码+音频解码+音视频同步
>在这里你可以看到全部的jni部分代码，不隐藏实现的c++代码，java调用c，然后c中回调java方法，在native层开启线程，同步锁pthred_mutex,pthread等等的使用。
>你只需要有一点c的基础就可以动手开始改造本项目，查看本项目的每一次日志提交你能勾有清晰的脉络知识结构认知。

# 为什么选用静态库
> 静态库是源代码的静态备份，jni编译时候会选择有用的头文件加载对应的源文件打包成so库，方便开发者写native层代码，
> 目前开源项目采用静态库的不是很多，极不方便新手入门jni和音视频开发，我就是要让你能够像写`hello-world`一样开始音视频的开发，that's all! 

![pic](https://github.com/jdpxiaoming/PPlayer/blob/master/capture/output2.gif)

# 欢迎给我提`issue`，有想法也可以提. 

# ToDoList
- [x] 视频解码
- [x] 音频解码
- [x] 音视频同步
- [x] 播放在线流媒体RTMP、RTSP、HLS
- [ ] 线程释放
- [ ] `seek`进度条拖动
- [ ] `libx264`推流`H264`
- [ ] `faac`推流`AAC` 
 

# 运行demo
1. 去release中下载源代码，然后看下面静态库地址可以考虑百度网盘
1. 直接使用AndroidStudio导入本项目运行app
2. 拷贝`capture/input.mp4`到`/data/com.poe.pplayer/cache`
3. 点击"播放"

# 本项目默认支持`rtsp`和`rtmp`的播放，只需要在项目中把文件地址更换为对应uri（已验证RTMP，其他地址未一一验证，如果无法播放，请提`issue`）.
```
mPlayer.setDataSource("rtmp://xxxxxxx.com/abcd12343");
```

# 测试流地址 
1. 推荐[red5百度网盘](https://pan.baidu.com/s/1IEbbWcg5633GkL0V5MTupw) 提取码：`veyq`
2. `nginx+lib-rtmp`服务器搭配`FFmpeg`或者`Obs`推流


# 静态库由于体积太大，我放到了百度网盘，
[FFmpeg4.0.2静态库（32/64bit)](https://pan.baidu.com/s/1Jh6HpRssMZTz2OH8j1GrGg) 提取码：`2c16`

# License
    Copyright 2020 Poe.Cai.

    FFmpeg codebase is mainly LGPL-licensed with optional components licensed under GPL. Please refer to the LICENSE file for detailed information.


