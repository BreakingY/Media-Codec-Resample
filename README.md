# Media-Codec-Resample
x264、x265、openh264、libde265、fdk_aac、G711、libyuv、speexdsp测试程序。

# x264
* 安装：
  - sudo apt install libx264-dev
* 编译运行：
  - g++ x264_yuv2h264.cpp -o x264-test -lx264
  - ./x264-test ../media/352_288.yuv out.h264 352 288 25

# x265
* 安装：
  - sudo apt install libx265-dev
* 编译运行：
  - g++ x265_yuv2h265.cpp -o x265-test -lx265
  - ./x265-test ../media/352_288.yuv out.h265 352 288 25

# openh264
* 安装：
  - git clone https://github.com/cisco/openh264.git
  - cd openh264
  - make -j
  - make install
* 解码：
  - g++ openh264_decode.cpp -o openh264-dec-test -lopenh264
  - ./openh264-dec-test ../media/352_288.h264 out.yuv
* 编码：
  - g++ openh264_encode.cpp -o openh264-enc-test -lopenh264
  - ./openh264-enc-test ../media/352_288.yuv out.h264 352 288

# libde265
* 安装：
  - apt install libqt4-dev
  - git clone https://github.com/strukturag/libde265.git
  - cd libde265
  - mkdir build
  - cd build
  - cmake -DCMAKE_BUILD_TYPE=Release ..
  - make -j
  - make install
* 编译运行：
  - g++ libde265_decode.cpp -o de265-test -lde265
  - ./de265-test ../media/352_288.h265 out.yuv

# libyuv
* 安装：
  - git clone https://github.com/lemenkov/libyuv.git
  - cd libyuv
  - make -f  linux.mk
* 编译运行：
  - g++ example.cpp -o example -I./include -L./ -lyuv
  - ./example ../media/352_288_pic.yuv 352 288

# fdk-aac
* 安装：
  - sudo apt install libfdk-aac-dev
* 解码：
  - g++ fdk_aac2pcm_decode.cpp -o fdk_aac_dec -lfdk-aac
  - ./fdk_aac_dec ../media/audio_44100_2_16.aac out.pcm 44100 2 16
* 编码：
  - g++ fdk_pcm2aac_encode.cpp -o fdk_aac_enc -lfdk-aac
  - ./fdk_aac_enc ../media/audio_44100_2_16.pcm out.aac 44100 2 16

# G711
G711源项目地址http://www.speech.kth.se/cost250/refsys/latest/src/
* 编译运行：
  - mkdir build
  - cd build
  - cmake ..
  - make -j
  - ./example ../../media/audio_44100_2_16.pcm pcm_ulaw out.g711

# speexdsp
* 安装：
  - sudo apt-get install -y autoconf automake libtool
  - 下载源码https://github.com/xiph/speexdsp/tags
  - 解压源码
  - cd speexdsp-SpeexDSP-1.2.1/
  - ./autogen.sh
  - ./configure --disable-oggtest
  - make
  - make install
* 编译运行：
  - g++ resample.cpp -o resample -lspeexdsp
  - ./resample ../media/audio_44100_2_16.pcm out.pcm 2 44100 16 2 8000 16

# 注意
* 项目里面的部分代码是我学习过程中从网上摘下来并修改的，如遇源代码作者可以给出源代码地址，我会在这里列出来

# 技术交流
* kxsun617@163.com

