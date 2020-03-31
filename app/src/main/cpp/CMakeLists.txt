cmake_minimum_required(VERSION 3.4.1)
file(GLOB SOURCE ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h)

add_library(
        pplayer
        SHARED
        ${SOURCE})

find_library(
        log-lib
        log )

#导入编译时头文件.
include_directories(${CMAKE_SOURCE_DIR}/include)
#设置链接静态库的目录.
set(my_lib_path ${CMAKE_SOURCE_DIR}/../../../libs/${CMAKE_ANDROID_ARCH_ABI})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${my_lib_path}")

target_link_libraries(
        pplayer
        avfilter avformat avcodec avutil swresample swscale
        android
        z
        OpenSLES
        ${log-lib} )