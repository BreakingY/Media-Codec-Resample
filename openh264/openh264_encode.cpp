#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <wels/codec_api.h>
#include <wels/codec_app_def.h>
#include <wels/codec_def.h>
#include <wels/codec_ver.h>
#define PRINT(arg) std::cout << #arg << "=" << arg << std::endl;
void ptintfExt(SEncParamExt &encParamExt)
{
    PRINT(encParamExt.iUsageType)

    PRINT(encParamExt.iPicWidth)
    PRINT(encParamExt.iPicHeight)
    PRINT(encParamExt.iTargetBitrate)
    PRINT(encParamExt.iRCMode)
    PRINT(encParamExt.fMaxFrameRate)

    PRINT(encParamExt.iTemporalLayerNum)
    PRINT(encParamExt.iSpatialLayerNum)

    SSpatialLayerConfig sSpatialLayers = encParamExt.sSpatialLayers[0];
    PRINT(sSpatialLayers.iVideoWidth)
    PRINT(sSpatialLayers.iVideoHeight)
    PRINT(sSpatialLayers.fFrameRate)
    PRINT(sSpatialLayers.iSpatialBitrate)
    PRINT(sSpatialLayers.iMaxSpatialBitrate)
    PRINT(sSpatialLayers.uiProfileIdc)
    PRINT(sSpatialLayers.uiLevelIdc)
    PRINT(sSpatialLayers.iDLayerQp)

    SSliceArgument sSliceArgument = sSpatialLayers.sSliceArgument;

    PRINT(sSliceArgument.uiSliceMode)
    PRINT(sSliceArgument.uiSliceNum)

    for (int i = 0; i < MAX_SLICES_NUM_TMP; i++) {
        PRINT(sSliceArgument.uiSliceMbNum[i])
    }
    PRINT(sSliceArgument.uiSliceSizeConstraint)

    PRINT(sSpatialLayers.bVideoSignalTypePresent)
    PRINT(sSpatialLayers.uiVideoFormat)
    PRINT(sSpatialLayers.bFullRange)
    PRINT(sSpatialLayers.bColorDescriptionPresent)
    PRINT(sSpatialLayers.uiColorPrimaries)
    PRINT(sSpatialLayers.uiTransferCharacteristics)
    PRINT(sSpatialLayers.uiColorMatrix)

    PRINT(sSpatialLayers.bAspectRatioPresent)
    PRINT(sSpatialLayers.eAspectRatio)
    PRINT(sSpatialLayers.sAspectRatioExtWidth)
    PRINT(sSpatialLayers.sAspectRatioExtHeight)

    PRINT(encParamExt.iComplexityMode)
    PRINT(encParamExt.uiIntraPeriod)
    PRINT(encParamExt.iNumRefFrame)
    PRINT(encParamExt.eSpsPpsIdStrategy)
    PRINT(encParamExt.bPrefixNalAddingCtrl)
    PRINT(encParamExt.bEnableSSEI)
    PRINT(encParamExt.bSimulcastAVC)
    PRINT(encParamExt.iPaddingFlag)
    PRINT(encParamExt.iEntropyCodingModeFlag)

    PRINT(encParamExt.bEnableFrameSkip)
    PRINT(encParamExt.iMaxBitrate)
    PRINT(encParamExt.iMaxQp)
    PRINT(encParamExt.iMinQp)
    PRINT(encParamExt.uiMaxNalSize)

    PRINT(encParamExt.bEnableLongTermReference)
    PRINT(encParamExt.iLTRRefNum)
    PRINT(encParamExt.iLtrMarkPeriod)

    PRINT(encParamExt.iMultipleThreadIdc)
    PRINT(encParamExt.bUseLoadBalancing)

    PRINT(encParamExt.iLoopFilterDisableIdc)
    PRINT(encParamExt.iLoopFilterAlphaC0Offset)
    PRINT(encParamExt.iLoopFilterBetaOffset)

    PRINT(encParamExt.bEnableDenoise)
    PRINT(encParamExt.bEnableBackgroundDetection)
    PRINT(encParamExt.bEnableAdaptiveQuant)
    PRINT(encParamExt.bEnableFrameCroppingFlag)
    PRINT(encParamExt.bEnableSceneChangeDetect)

    PRINT(encParamExt.bIsLosslessLink)
    PRINT(encParamExt.bFixRCOverShoot)
    PRINT(encParamExt.iIdrBitrateRatio)
}
SEncParamBase encoderParam;
SEncParamExt encoderParamExt;
char *srcFile;
char *dstFile;
int picWidth;
int picHeight;
ISVCEncoder *encoder;
unsigned char *data;
// 实时视频流编码可以不设置帧率
void InitParam(SEncParamBase &param)
{
    memset(&param, 0, sizeof(SEncParamBase));
    param.iUsageType = CAMERA_VIDEO_REAL_TIME; // 用于实时通信的摄像头
    // param.fMaxFrameRate = 25;
    param.iPicWidth = picWidth;
    param.iPicHeight = picHeight;
    param.iRCMode = RC_BITRATE_MODE;
    /*RC_QUALITY_MODE: 质量模式，即根据目标质量来控制视频编码的比特率。
    RC_BITRATE_MODE: 比特率模式，即根据目标比特率来控制视频编码的质量。
    RC_BUFFERBASED_MODE: 基于缓冲区的模式，不使用比特率控制，只根据缓冲区状态来调整视频质量。
    RC_TIMESTAMP_MODE: 基于时间戳的速率控制模式，可能根据时间戳信息来控制编码。
    RC_BITRATE_MODE_POST_SKIP: 此模式可能是正在进行算法调优时使用的临时模式，这个模式将被删除。
    RC_OFF_MODE: 关闭速率控制，即不进行任何速率控制。
    */
    param.iTargetBitrate = 2048 * 1024; // 单位是bps,不是kbps
}
void InitParamExt(SEncParamExt &param)
{
    memset(&param, 0, sizeof(SEncParamExt));
    param.iUsageType = CAMERA_VIDEO_REAL_TIME;
    param.iPicWidth = picWidth;
    param.iPicHeight = picHeight;
    param.iRCMode = RC_BITRATE_MODE;
    param.iTargetBitrate = 2048 * 1024;  // 单位是bps,不是kbps
    param.iMaxBitrate = 2048 * 1024 * 2; // 单位是bps,不是kbps

    // param.fMaxFrameRate = 150;
    param.uiIntraPeriod = 50; // I帧间隔
    param.iNumRefFrame = 0;   // B帧数量
    /*
    typedef enum {
        LOW_COMPLEXITY = 0,              ///< the lowest compleixty,the fastest speed, 对应Baseline Profile
        MEDIUM_COMPLEXITY,          ///< medium complexity, medium speed,medium quality 对应Main Profile
        HIGH_COMPLEXITY             ///< high complexity, lowest speed, high quality 对应High Profile
    } ECOMPLEXITY_MODE;
    */
    param.iComplexityMode = LOW_COMPLEXITY;
    param.bEnableFrameSkip = false; // 跳帧，编码器是否允许在特定情况下跳过一些帧的编码，以便在保持目标比特率范围内
    /*
    typedef enum {
        CONSTANT_ID = 0,           ///< constant id in SPS/PPS 整个视频序列中SPS和PPS都保持相同的ID，视频只有一个sps pps
        INCREASING_ID = 0x01,      ///< SPS/PPS id increases at each IDR 每个IDR新增sps pps
        SPS_LISTING  = 0x02,       ///< using SPS in the existing list if possible
        SPS_LISTING_AND_PPS_INCREASING  = 0x03,
        SPS_PPS_LISTING  = 0x06,
    } EParameterSetStrategy;
    */
    // 整个视频序列中SPS和PPS都保持相同的ID，可以选择常量ID策略。而如果希望在每个视频帧中都有新的SPS和PPS，可以选择附加和映射-6和附加策略-1。
    param.eSpsPpsIdStrategy = CONSTANT_ID;

    param.bEnableSSEI = false; // 是否启添加SEI帧
    // param.iEntropyCodingModeFlag = 1;      ///< 0:CAVLC  1:CABAC.

    // 设置QP，可以根据自己的需要来，QP越大码率越小（图像的质量越差）
    // param.iMaxQp = 38;
    // param.iMinQp = 16;

    // iTemporalLayerNum=1 iSpatialLayerNum=1 没有svc
    param.iTemporalLayerNum = 1;
    param.iSpatialLayerNum = 1;
    param.bEnableDenoise = false;
    param.bEnableBackgroundDetection = true; // 背景检测
    param.bEnableSceneChangeDetect = true;   // 场景变化检测
    param.bEnableAdaptiveQuant = false;      // 自适应质量控制
    param.bEnableFrameSkip = false;
    param.bEnableLongTermReference = false;
    param.iPaddingFlag = 0; // padding

    param.bPrefixNalAddingCtrl = false;
    param.sSpatialLayers[0].iVideoWidth = picWidth;
    param.sSpatialLayers[0].iVideoHeight = picHeight;
    // param.sSpatialLayers[0].fFrameRate = 25;
    param.sSpatialLayers[0].iSpatialBitrate = param.iTargetBitrate;
    param.sSpatialLayers[0].iMaxSpatialBitrate = param.iMaxBitrate;
    param.sSpatialLayers[0].uiProfileIdc = PRO_BASELINE;
    param.sSpatialLayers[0].uiLevelIdc = LEVEL_UNKNOWN; // 自动检测
    param.sSpatialLayers[0].uiVideoFormat = VF_PAL;     // pal 0-5 => component, kpal, ntsc, secam, mac, undef

    param.bEnableFrameCroppingFlag = 1; // 必须设置为1，否则如果是1080的编码后的尺寸就会变成1088(视频缓冲区大小)，x264 x265不需要配置，应该是默认的
}
void SetupEncoder()
{
    /*
    openh264提供了两个结构体来设置编码参数，SEncParamBase与SEncParamExt SEncParamBase仅通过了最基础的参数设置
    */
    int rv = WelsCreateSVCEncoder(&encoder);
    assert(0 == rv);
    assert(encoder != NULL);
#if 0
    /*SEncParamBase方式初始化解码器*/
    InitParam(encoderParam);
    assert(encoder->Initialize(&encoderParam) == 0);
#else
    /*SEncParamExt方式初始化编码器*/
    // 获取默认参数
    encoder->GetDefaultParams(&encoderParamExt);
    InitParamExt(encoderParamExt);
    assert(encoder->InitializeExt(&encoderParamExt) == 0);
#endif

#if 0
    //changing logging granularity 日志
    int g_LevelSetting = 1;
    encoder->SetOption(ENCODER_OPTION_TRACE_LEVEL, &g_LevelSetting);

    int videoFormat = videoFormatI420;//输入格式
    encoder->SetOption(ENCODER_OPTION_DATAFORMAT, &videoFormat);

    //changing encoding param
    //encoder->SetOption(ENCODER_OPTION_SVC_ENCODE_PARAM_BASE, &param);
#endif
}
void Encode()
{
    const char *file_name = srcFile;
    FILE *inFd = fopen(srcFile, "r+");
    FILE *outFd = fopen(dstFile, "w+");
    assert(outFd != nullptr);

    SFrameBSInfo info;  // 解码后的数据
    SSourcePicture pic; // 需要编码的图像
    pic.iPicWidth = picWidth;
    pic.iPicHeight = picHeight;
    pic.iColorFormat = videoFormatI420;
    pic.iStride[0] = pic.iPicWidth;
    pic.iStride[1] = pic.iStride[2] = pic.iPicWidth / 2;
    pic.pData[0] = data;
    pic.pData[1] = pic.pData[0] + picWidth * picHeight;
    pic.pData[2] = pic.pData[1] + (picWidth * picHeight / 4);
    SEncParamExt encParamExt;
    encoder->GetOption(ENCODER_OPTION_SVC_ENCODE_PARAM_EXT, &encParamExt);
    ptintfExt(encParamExt);

    int frame_num = 0;
    int total_bytes = 0;
    while (frame_num <= 60) {
        int frame_size = picWidth * picHeight;
        frame_size = picWidth * picHeight * 3 / 2;
        if (fread(data, 1, frame_size, inFd) <= 0) {
            std::cout << "file over" << std::endl;
            break;
        }
        /*
        if(frame_num == 100){
            std::cout<<"change bitrate to 100kb/s"<<std::endl;
            encParamExt.sSpatialLayers[0].iSpatialBitrate =  100 * 1000;
            encParamExt.sSpatialLayers[0].iMaxSpatialBitrate =  100 * 1000;
            encoder->SetOption(ENCODER_OPTION_SVC_ENCODE_PARAM_EXT, &encParamExt);
        }
        */
        int rv = encoder->EncodeFrame(&pic, &info);
        assert(rv == cmResultSuccess);
        std::cout << "----------uiTimeStamp : " << info.uiTimeStamp << std::endl;
        fwrite(info.sLayerInfo[0].pBsBuf, 1, info.iFrameSizeInBytes, outFd); // with start code
        //++frame_num;

#if 0
        //svc 待学习
        if (info.eFrameType != videoFrameTypeSkip)
        {   
            
            int iLayerNum = info.iLayerNum; 
            printf("iLayerNum:%d\n",iLayerNum);
            for(int i = 0;i<iLayerNum;++i){
                SLayerBSInfo &layer = info.sLayerInfo[i];
                int iLayerSize = 0;
                int iLNalIdx = layer.iNalCount - 1;
                do
                {
                    iLayerSize += layer.pNalLengthInByte[iLNalIdx];
                } while (--iLNalIdx >= 0);
                fwrite(layer.pBsBuf, 1, iLayerSize, outFd);//with start code
                ++frame_num;
                total_bytes += iLayerSize;
                int type=layer.pBsBuf[4]&0x1f;
                std::cout << "save frame: " << frame_num << ", size:" << iLayerSize << ", uiTimeStamp:" << info.uiTimeStamp << ", Type:"<< type <<std::endl;
                //printf("layer.pBsBuf:%0x/n",layer.pBsBuf[0]);
                //printf("layer.pBsBuf:%0x/n",layer.pBsBuf[1]);
                //printf("layer.pBsBuf:%0x/n",layer.pBsBuf[2]);
                //printf("layer.pBsBuf:%0x/n",layer.pBsBuf[3]);
            }
        }
        else{
            std::cout<<"videoFrameTypeskip for rate control"<<std::endl;
        }
#endif
    }
    std::cout << "save frame to file: \"" << dstFile << "\", total bytes: " << total_bytes << std::endl;
    fflush(outFd);
    fclose(outFd);
    fclose(inFd);
}
int main(int argc, char **argv)
{
    if (argc < 5) {
        printf("./bin input ouput width height\n");
        return -1;
    }
    srcFile = argv[1];
    dstFile = argv[2];
    picWidth = atoi(argv[3]);
    picHeight = atoi(argv[4]);
    int frame_size = picWidth * picHeight * 3 / 2;
    data = new unsigned char[frame_size];
    SetupEncoder();
    Encode();
    encoder->Uninitialize();
    WelsDestroySVCEncoder(encoder);
    delete data;
    return 0;
}
