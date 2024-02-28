#include <assert.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <wels/codec_api.h>
#include <wels/codec_app_def.h>
#include <wels/codec_def.h>
#include <wels/codec_ver.h>
static inline int startCode3(unsigned char *buf)
{
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return 1;
    else
        return 0;
}

static inline int startCode4(unsigned char *buf)
{
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return 1;
    else
        return 0;
}

static unsigned char *findNextStartCode(unsigned char *buf, int len)
{
    int i;

    if (len < 3)
        return NULL;

    for (i = 0; i < len - 3; ++i) {
        if (startCode3(buf) || startCode4(buf))
            return buf;

        ++buf;
    }

    if (startCode3(buf))
        return buf;

    return NULL;
}

static int getFrameFromH264File(FILE *file, unsigned char *frame, int size)
{
    int rSize, frameSize;
    unsigned char *nextStartCode;

    rSize = fread(frame, 1, size, file);
    if (!startCode3(frame) && !startCode4(frame))
        return -1;

    nextStartCode = findNextStartCode(frame + 3, rSize - 3);
    if (!nextStartCode) {
        // fseek(file, 0, SEEK_SET);
        // frameSize = rSize;
        return -1;
    } else {
        frameSize = (nextStartCode - frame);
        fseek(file, frameSize - rSize, SEEK_CUR);
    }

    return frameSize;
}
int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("./bin input ouput\n");
        return -1;
    }
    printf("decode start\n");
    char *srcFile = argv[1];
    char *dstFile = argv[2];

    FILE *inFile = fopen(argv[1], "rb");
    if (!inFile) {
        printf("failed to open %s\n", argv[1]);
        return -1;
    }
    FILE *outFile = fopen(argv[2], "wb");
    if (!outFile) {
        printf("failed to open %s\n", argv[2]);
        fclose(inFile);
        return -1;
    }
    int len = 4 * 1024 * 1024;
    unsigned char *frame = (unsigned char *)malloc(len);
    ISVCDecoder *decoder = nullptr;
    int ret = WelsCreateDecoder(&decoder);
    assert(ret == 0);
    SDecodingParam dparam = {0};
    dparam.sVideoProperty.size = sizeof(dparam.sVideoProperty);
    dparam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
    dparam.eEcActiveIdc = ERROR_CON_SLICE_COPY;
    /*
    如果输入视频是一个多层的可伸缩视频编码（SVC）视频流，设置`uiTargetDqLayer`为`UCHAR_MAX`将允许解码器解码所有的图层从而获得整个视频的完整解码结果。这在一些应用场景中可能是有用的，比如需要对视频进行全层级的处理、分析或转码。
    解码更多的图层可能会增加解码器的计算负担和内存使用，实际应用中需要根据系统资源和需求合理设置`uiTargetDqLayer`值，如果只需要解码特定的几个图层，可以将`uiTargetDqLayer`设置为相应的图层ID，而不是使用`UCHAR_MAX`。
    */
    dparam.uiTargetDqLayer = UCHAR_MAX; // unsigned char的最大值 255
    dparam.bParseOnly = false;
    ret = decoder->Initialize(&dparam);
    assert(ret == 0);

    int frame_num = 0;
    while (true) {
        int frameSize = getFrameFromH264File(inFile, frame, len);
        if (frameSize < 0) {
            printf("file over\n");
            break;
        }
        int type;
        if (startCode3(frame)) {
            type = frame[3] & 0x1f;
        } else if (startCode4(frame)) {
            type = frame[4] & 0x1f;
        } else {
            printf("start code error\n");
        }
        printf("type:%d\n", type);
        unsigned char *dst[3] = {0};
        SBufferInfo dinfo = {0}; // 需要使用局部变量，否则上一次的解码状态没有清空会有影响
        ret = decoder->DecodeFrameNoDelay(frame, frameSize, dst, &dinfo);
        if (ret >= 0 && dinfo.iBufferStatus == 1 && dinfo.UsrData.sSystemBuffer.iFormat == videoFormatI420) {
            printf("width:%d height:%d dinfo.UsrData.sSystemBuffer.iStride[0]:%d\n", dinfo.UsrData.sSystemBuffer.iWidth, dinfo.UsrData.sSystemBuffer.iHeight, dinfo.UsrData.sSystemBuffer.iStride[0]);
            printf("width:%d height:%d dinfo.UsrData.sSystemBuffer.iStride[1]:%d\n", dinfo.UsrData.sSystemBuffer.iWidth, dinfo.UsrData.sSystemBuffer.iHeight, dinfo.UsrData.sSystemBuffer.iStride[1]);
            printf("width:%d height:%d dinfo.UsrData.sSystemBuffer.iStride[2]:%d\n", dinfo.UsrData.sSystemBuffer.iWidth, dinfo.UsrData.sSystemBuffer.iHeight, dinfo.UsrData.sSystemBuffer.iStride[2]);
            int rows;
            // dst[i]=data+padding
            // Y
            rows = dinfo.UsrData.sSystemBuffer.iHeight;
            for (int row = 0; row < rows; row++) {
                fwrite((char *)dst[0] + dinfo.UsrData.sSystemBuffer.iStride[0] * row, 1, dinfo.UsrData.sSystemBuffer.iWidth, outFile);
            }
            // U
            rows = dinfo.UsrData.sSystemBuffer.iHeight / 2;
            for (int row = 0; row < rows; row++) {
                fwrite((char *)dst[1] + dinfo.UsrData.sSystemBuffer.iStride[1] * row, 1, dinfo.UsrData.sSystemBuffer.iWidth / 2, outFile);
            }
            // V
            rows = dinfo.UsrData.sSystemBuffer.iHeight / 2;
            for (int row = 0; row < rows; row++) {
                int v_stride = dinfo.UsrData.sSystemBuffer.iStride[2];
                if (dinfo.UsrData.sSystemBuffer.iStride[2] == 0 || dinfo.UsrData.sSystemBuffer.iStride[2] != dinfo.UsrData.sSystemBuffer.iStride[1]) {
                    // int iStride[2];                ///< stride of 2 component,openh264只保存两个分量的跨度，U V是相等的，iStride数组里面只有两个值 iStride[0]为Y跨度 iStride[1]为U V跨度 iStride[2]没用
                    v_stride = dinfo.UsrData.sSystemBuffer.iStride[1];
                }
                fwrite((char *)dst[2] + v_stride * row, 1, dinfo.UsrData.sSystemBuffer.iWidth / 2, outFile);
            }
            fflush(outFile);
            frame_num++;
        }
    }
    // flush
    auto left = decoder->GetOption(DECODER_OPTION_NUM_OF_FRAMES_REMAINING_IN_BUFFER, nullptr);
    std::cout << "left:" << left << std::endl;
    while (left-- > 0) {
        printf("left:%d\n", left);
        unsigned char *dst[3] = {0};
        SBufferInfo dinfo = {0}; // 需要使用局部变量，否则上一次的解码状态没有清空会有影响
        int ret = decoder->FlushFrame(dst, &dinfo);
        if (ret >= 0 && dinfo.iBufferStatus == 1 && dinfo.UsrData.sSystemBuffer.iFormat == videoFormatI420) {
            printf("dst[0]:%p %d\n", dst[0], __LINE__);
            printf("dst[1]:%p %d\n", dst[1], __LINE__);
            printf("dst[2]:%p %d\n", dst[2], __LINE__);
            int rows;
            // dst[i]=data+padding
            // Y
            rows = dinfo.UsrData.sSystemBuffer.iHeight;
            for (int row = 0; row < rows; row++) {
                fwrite((char *)dst[0] + dinfo.UsrData.sSystemBuffer.iStride[0] * row, 1, dinfo.UsrData.sSystemBuffer.iWidth, outFile);
            }
            // U
            rows = dinfo.UsrData.sSystemBuffer.iHeight / 2;
            for (int row = 0; row < rows; row++) {
                fwrite((char *)dst[1] + dinfo.UsrData.sSystemBuffer.iStride[1] * row, 1, dinfo.UsrData.sSystemBuffer.iWidth / 2, outFile);
            }
            // V
            rows = dinfo.UsrData.sSystemBuffer.iHeight / 2;
            for (int row = 0; row < rows; row++) {
                int v_stride = dinfo.UsrData.sSystemBuffer.iStride[2];
                if (dinfo.UsrData.sSystemBuffer.iStride[2] == 0 || dinfo.UsrData.sSystemBuffer.iStride[2] != dinfo.UsrData.sSystemBuffer.iStride[1]) {
                    v_stride = dinfo.UsrData.sSystemBuffer.iStride[1];
                }
                fwrite((char *)dst[2] + v_stride * row, 1, dinfo.UsrData.sSystemBuffer.iWidth / 2, outFile);
            }
            fflush(outFile);
        } else {
            printf("FlushFrame ret:%0x\n", ret);
        }
    }
    decoder->Uninitialize();
    WelsDestroyDecoder(decoder);
    decoder = nullptr;
    fclose(outFile);
    fclose(inFile);
    free(frame);
    return 0;
}