# openh264解码
typedef enum {
  ERROR_CON_DISABLE = 0,
  ERROR_CON_FRAME_COPY,
  ERROR_CON_SLICE_COPY,
  ERROR_CON_FRAME_COPY_CROSS_IDR,
  ERROR_CON_SLICE_COPY_CROSS_IDR,
  ERROR_CON_SLICE_COPY_CROSS_IDR_FREEZE_RES_CHANGE,
  ERROR_CON_SLICE_MV_COPY_CROSS_IDR,
  ERROR_CON_SLICE_MV_COPY_CROSS_IDR_FREEZE_RES_CHANGE
} ERROR_CON_IDC;
typedef struct {
  unsigned int          size;          ///< size of the struct
  VIDEO_BITSTREAM_TYPE  eVideoBsType;  ///< video stream type (AVC/SVC)
} SVideoProperty;
typedef enum {
  VIDEO_BITSTREAM_AVC               = 0,
  VIDEO_BITSTREAM_SVC               = 1,
  VIDEO_BITSTREAM_DEFAULT           = VIDEO_BITSTREAM_SVC
} VIDEO_BITSTREAM_TYPE;
typedef struct TagSVCDecodingParam {
  char*     pFileNameRestructed;       ///<  用于计算PSNR（峰值信噪比）的调试目的，指定重建帧（解码后的图像）的文件名。

  unsigned int  uiCpuLoad;             ///< 解码器的CPU负载设置，表示解码器在解码过程中使用的CPU资源的多少。数值越大，解码器使用的CPU资源越多，解码速度可能更快，但也可能对系统的其他任务产生影响。通常，这个参数是一个0到100的整数值。s
  unsigned char uiTargetDqLayer;       ///< 设置解码器的目标降频图层（DQ Layer）。OpenH264支持可伸缩视频编码（SVC），其中视频可以被分解成多个图层，每个图层具有不同的空间分辨率或编码质量。该字段用于指定解码器解码的目标图层的ID。

  ERROR_CON_IDC eEcActiveIdc;          ///< 控制解码器是否启用错误掩盖功能。错误掩盖是一种在解码过程中处理丢失或损坏数据的技术，用于尽可能地恢复视频质量。该字段用于指定错误掩盖的级别
  bool bParseOnly;                     ///< 表示解码器是否仅进行解析而不进行重构（解码后的图像）。当设置为`true`时，解码器只解析输入的比特流，不进行图像重构，
                                       ///< 这对于一些特定的应用场景可能很有用。需要注意的是，当该字段为`true`时，SPS（序列参数集）和PPS（图像参数集）的大小不能超过`SPS_PPS_BS_SIZE`（128字节），否则会返回错误信息。
  SVideoProperty   sVideoProperty;    ///< 用于指定视频的一些基本属性，如图像宽度、高度、颜色格式等信息。
} SDecodingParam, *PDecodingParam;

typedef struct TagBufferInfo {
    int iBufferStatus;             ///< 0: one frame data is not ready; 1: one frame data is ready
    unsigned long long uiInBsTimeStamp;     ///< input BS timestamp
    unsigned long long uiOutYuvTimeStamp;     ///< output YUV timestamp, when bufferstatus is 1
    union {
        SSysMEMBuffer sSystemBuffer; ///<  memory info for one picture
    } UsrData;                     ///<  output buffer info
    unsigned char* pDst[3];  //point to picture YUV data
} SBufferInfo;
typedef struct TagSysMemBuffer {
    int iWidth;                    ///< width of decoded pic for display
    int iHeight;                   ///< height of decoded pic for display
    int iFormat;                   ///< type is "EVideoFormatType"
    int iStride[2];                ///< stride of 2 component iStride里面不全都是图像，包括填充
} SSysMEMBuffer;
# openh264编码

typedef struct TagEncParamExt {
  EUsageType iUsageType;                          ///< 使用类型，与TagEncParamBase中的相同
  int iPicWidth;                                  ///< 图像宽度，与TagEncParamBase中的相同
  int iPicHeight;                                 ///< 图像高度，与TagEncParamBase中的相同
  int iTargetBitrate;                             ///< 目标比特率，与TagEncParamBase中的相同
  RC_MODES iRCMode;                               ///< 码率控制模式，与TagEncParamBase中的相同
  float fMaxFrameRate;                            ///< 最大帧率，与TagEncParamBase中的相同

  int iTemporalLayerNum;                          ///< 时序层数量，最大时序层=4
  int iSpatialLayerNum;                           ///< 空间层数量，1<= iSpatialLayerNum <= MAX_SPATIAL_LAYER_NUM, MAX_SPATIAL_LAYER_NUM = 4
  SSpatialLayerConfig sSpatialLayers[MAX_SPATIAL_LAYER_NUM]; ///< 空间层配置信息

  ECOMPLEXITY_MODE iComplexityMode;               ///< 复杂度模式
  unsigned int uiIntraPeriod;                     ///< Intra帧的周期
  int iNumRefFrame;                               ///< 参考帧数目
  EParameterSetStrategy eSpsPpsIdStrategy;        ///< SPS/PPS标识的不同策略：0-恒定ID，1-附加ID，6-映射和附加
  bool bPrefixNalAddingCtrl;                      ///< 是否使用Prefix NAL
  bool bEnableSSEI;                               ///< 是否使用SSEI（Supplemental Enhancement Information）-- TODO：计划移除SSEI接口
  bool bSimulcastAVC;                             ///< (编码多个空间层时) false: 使用SVC语法进行更高层编码; true: 使用Simulcast AVC
  int iPaddingFlag;                               ///< 0：禁用填充; 1：填充
  int iEntropyCodingModeFlag;                     ///< 0：CAVLC; 1：CABAC。

  /* RC 控制 */
  bool bEnableFrameSkip;                          ///< False：即使VBV缓冲溢出也不跳过帧。True：允许跳过帧以保持比特率在限制范围内
  int iMaxBitrate;                                ///< 最大比特率，单位bps，如果不需要设置为UNSPECIFIED_BIT_RATE
  int iMaxQp;                                     ///< 编码器支持的最大QP值
  int iMinQp;                                     ///< 编码器支持的最小QP值
  unsigned int uiMaxNalSize;                      ///< 最大NAL大小。对于动态切片模式，此值不应为0

  /*LTR 设置*/
  bool bEnableLongTermReference;                  ///< 1：启用长期参考帧，0：禁用
  int iLTRRefNum;                                 ///< 长期参考帧的数量。TODO: 目前不支持任意设置
  unsigned int iLtrMarkPeriod;                    ///< 在反馈中使用的LTR标记周期。
  /* 多线程设置 */
  unsigned short iMultipleThreadIdc;              ///< 1 # 0: 自动（动态实现内部编码器）; 1: 禁用多线程实现; 大于1: 线程数
  bool bUseLoadBalancing;                         ///< 仅在uiSliceMode=1或3时使用，会在多线程编码的运行时改变图像的切片，因此每次运行的结果可能不同

  /* Deblocking 循环滤波器 */
  int iLoopFilterDisableIdc;                      ///< 0: 启用循环滤波器; 1: 禁用循环滤波器; 2: 除了切片边界以外都启用循环滤波器
  int iLoopFilterAlphaC0Offset;                   ///< AlphaOffset: 有效范围[-6, 6]，默认0
  int iLoopFilterBetaOffset;                      ///< BetaOffset: 有效范围[-6, 6]，默认0
  /* 预处理特性 */
  bool bEnableDenoise;                            ///< 降噪控制
  bool bEnableBackgroundDetection;                ///< 背景检测控制
  bool bEnableAdaptiveQuant;                      ///< 自适应量化控制
  bool bEnableFrameCroppingFlag;                  ///< 启用图像裁剪标志：始终为TRUE
  bool bEnableSceneChangeDetect;                  ///< 场景变化检测控制

  bool bIsLosslessLink;                           ///< LTR 高级设置
  bool bFixRCOverShoot;                           ///< 修正率控制过冲
  int iIdrBitrateRatio;                          ///< IDR目标比特数为(idr_bitrate_ratio/100) * 平均每帧目标比特数
} SEncParamExt;


typedef enum {
    videoFrameTypeInvalid,    ///< encoder not ready or parameters are invalidate
    videoFrameTypeIDR,        ///< IDR frame in H.264
    videoFrameTypeI,          ///< I frame type
    videoFrameTypeP,          ///< P frame type
    videoFrameTypeSkip,       ///< skip the frame based encoder kernel
    videoFrameTypeIPMixed     ///< a frame where I and P slices are mixing, not supported yet
} EVideoFrameType;
typedef struct {
    unsigned char uiTemporalId;//视频层的时域ID，用于表示编码的时域层次。
    unsigned char uiSpatialId;//视频层的空间ID，用于表示编码的空间层次。
    unsigned char uiQualityId;//视频层的质量ID，用于表示编码的质量层次。
    EVideoFrameType eFrameType;//视频帧的类型，可以是I帧、P帧或B帧。
    unsigned char uiLayerType;//视频层的类型，可以是SPS（Sequence Parameter Set）、PPS（Picture Parameter Set）或视频帧数据。

    /**
     * The sub sequence layers are ordered hierarchically based on their dependency on each other so that any picture in a layer shall not be
     * predicted from any picture on any higher layer.
     */
    int   iSubSeqId;                ///< refer to D.2.11 Sub-sequence information SEI message semantics 视频层所属的子序列ID，用于按照依赖关系对视频层进行层次化排序。
    int   iNalCount;              ///< count number of NAL coded already 该视频层中NAL单元的数量（NAL单元是H.264/AVC比特流的基本单元）。
    int*  pNalLengthInByte;       ///< length of NAL size in byte from 0 to iNalCount-1 指向一个整型数组，表示每个NAL单元的长度（以字节为单位）。
    unsigned char*  pBsBuf;       ///< buffer of bitstream contained 指向一个无符号字符数组，存储该视频层的比特流数据。
} SLayerBSInfo, *PLayerBSInfo;
//在OpenH264中，视频层是指在SVC（Scalable Video Coding）编码中的一个重要概念。SVC允许将一幅视频分解成多个视频层，每个视频层包含不同的编码质量或空间分辨率。
//这使得在传输或存储视频时，可以根据需要选择性地发送或接收不同的视频层，以实现逐步增加或减少视频质量或分辨率的效果。
typedef struct {
    int           iLayerNum;//该帧数据包含的视频层数量
    SLayerBSInfo  sLayerInfo[MAX_LAYER_NUM_OF_FRAME];//存储了每个视频层的信息，包括每个层的比特流数据和相关属性 MAX_LAYER_NUM_OF_FRAME=128

    EVideoFrameType eFrameType;//视频帧的类型，可以是I帧、P帧或B帧。
    int iFrameSizeInBytes;//编码后的视频帧数据的大小，以字节为单位。
    long long uiTimeStamp;//视频帧的时间戳，表示该帧在视频序列中的时间位置。
} SFrameBSInfo, *PFrameBSInfo;