# H264编码

h264编码过程：
1、预测(无损  帧内预测->残差数据，帧间预测(运动估计->计算运动矢量->补偿数据))
2、dct变换(无损 空域转换为频域)
3、量化(有损)
3-1、滤波，去除块处理产生的快效应
4、Z字扫描，使得相同的数字连接在一起然后进行熵编码(无损 霍夫曼编码（Huffman Coding）、上下文自适应二进制算术编码（Context Adaptive Binary Arithmetic Coding，CABAC）)


码流控制：
根据带宽，缓冲区来为编码图像分配一定的目标比特数，根据目标比特数和失真确定量化步长QP，由QP去求预测模式，求得预测模式，就可以知道实际的编码比特
利用这个实际的编码比特去更新缓冲区和下一帧分配的目标比特。QP小，编出来比特数多了，那么下一帧就分配少了，下一帧QP相应会加大，让编出来的比特率就少点。这样一个动态调节的过程。

****选择RDO为代价函数，对每种预测模式计算RDO，选择RDO最小的。需要QP是已知 在可选模式中选择最佳的模式，使得在限定码率 Rc下，失真 D 最小(预测编码失真是通过预测块和原始块差值的平方和来度量的)
    预测模式判决需要用到QP， 但是QP值只有码率控制完之后才知道，而码率控制要能够进行，就需要知道 MAD （宏块的失真，平均绝对差值）。MAD只有在预测模式知道后才能求得。
H264码率控制的基本步骤
1.  根据流体流量模型计算当前帧的目标比特率；
2.  将剩余比特平均分配到当前帧中所有未编码基本单元；
3.  根据前一帧中对应位置处基本单元的实际 MAD 值，用线性 MAD 预测模型来预测当前帧中当前基本单元的 MAD；
4.  利用二次 R-D 模型计算相应参数(QP)；
5.  用步骤 4 中得到的 QP 对当前基本单元中每个 MB 进行 RDO。获取预测模式
CBR:(固定比特率) 实时视频传输、视频直播或需要精确控制存储大小的应用
VBR:(动态比特率 1-pass和2-pass) 存储视频文件或在线视频服务 支持模式
ABR:(平均比特率，是VBR的一种插值参数,静态的图像分配低于平均码率的码率，对于复杂的，大量运动的图像分配高于平均码率的码流) 对网络传输带宽有一定要求的应用，能够平衡码率和视频质量
CVBR:(兼顾了CBR和VBR的优点 输入最大码率和最小码率，静止时码率稳定在最小码率，运动时利用前期节省的带宽来尽可能的提高图像质量,码率大于最小码率，但是又不超过最大码率)
CRF:质量恒定
CQP:(1-pass和2-pass) 恒定量化参数


1. 1-pass模式：
   视频编码器只进行单次编码过程。它会根据一些预设的参数和策略，尽可能地将视频数据压缩成目标比特率或达到特定的质量要求。适用于实时视频传输或对编码时间要求较高的场景。

2. 2-pass模式：
   视频编码器进行两次编码过程。第一遍编码（pass）会收集有关视频内容的统计信息，包括运动矢量、DCT系数分布、像素误差等
   然后，在第一遍编码的基础上，编码器进行第二遍编码，根据第一遍的数据来调整输出码率和优化视频质量。
   2-pass模式通常比1-pass模式更为精确，因为第一遍编码收集到的数据可以用来更好地调整码率和质量。2-pass模式适用于对视频质量要求较高的应用，例如视频存储、在线视频服务等。


x264支持的三种码率控制方式
X264_RC_CQP                  
X264_RC_CRF                  
X264_RC_ABR

# x264参数说明
typedef struct x264_param_t
{
    /* CPU 标志位 */
    unsigned int cpu;
    int         i_threads;           // 并行编码多帧; 线程数，为0则自动多线程编码
    int         i_lookahead_threads; /* multiple threads for lookahead analysis */
    int         b_sliced_threads;   // 如果为false，则一个slice只编码成一个NALU; 否则有几个线程，在编码成几个NALU。缺省为true。
    int         b_deterministic;  // 是否允许非确定性时线程优化
    int         b_cpu_independent; // 强制采用典型行为，而不是采用独立于cpu的优化算法
    int         i_sync_lookahead; // 线程超前缓存帧数

    /* 视频属性 */
    int         i_width; // 视频宽
    int         i_height; // 视频高
    int         i_csp;    //颜色空间 X264_CSP_I420
    int         i_level_idc; // level x264_param_t里并没有profile这个成员变量，因为不同的profile是描述有什么功能的，所以x264_param_apply_profile中实际是根据不同的profile名字修改x264_param_t里面的参数
    int         i_frame_total; // 编码帧的总数, 默认 0

    /* hrd : hypothetical reference decoder (假定参考解码器) , 检验编码器产生的符合该标准的NAL单元流或字节流的偏差值。蓝光视频、电视广播及其它特殊领域有此要求 */
    int         i_nal_hrd;
    /* vui参数集 : 视频可用性信息、视频标准化选项 位于sps中*/
    struct
    {
        /* they will be reduced to be 0 < x <= 65535 and prime */
        int         i_sar_height;  // 样本宽高比的高度
        int         i_sar_width; // 样本宽高比的宽度

        int         i_overscan;    /* 0=undef, 1=no overscan, 2=overscan 过扫描是视频显示的一种技术，为了兼容老式的电视或显示设备，会在视频边缘留出一定的空白区域，使视频内容不会被截断或被边缘覆盖。这样可以确保视频的全部内容在老式的显示设备上都能完整显示。*/

        /* see h264 annex E for the values of the following 下面的含义在附录E中*/
        int         i_vidformat;// 视频格式 视频在编码/数字化之前是什么类型(制式)，取值有：Component, PAL, NTSC, SECAM, MAC 等
        int         b_fullrange;   //是用来设置视频像素值的范围的选项。常见的两种范围是 "limited range" 和 "full range"。
                                   //Limited Range：也称为标准范围（standard range），通常用于视频传输和存储。像素值的范围被限制在 16-235（对于 8-bit YUV 视频），即黑色对应的像素值为 16，白色对应的像素值为 235。
                                    //Full Range：也称为扩展范围（expanded range），像素值的范围扩展至 0-255（对于 8-bit YUV 视频），即黑色对应的像素值为 0，白色对应的像素值为 255。
        int         i_colorprim; //原始色度格式 设置视频的色度原色信息的选项。色度原色（color primaries）是视频色彩空间中表示色彩范围的一组参数。不同的视频标准和显示设备可能使用不同的色度原色来表示颜色信息
                                //常见的色度原色包括：
                                //BT.709：高清电视的色度原色，用于HDTV视频。
                                //BT.601：标准电视的色度原色，用于SDTV视频。
                                //BT.2020：Ultra HD（UHD）和高动态范围（HDR）视频的广色域色度原色。
        int         i_transfer; //转换方式 数值含义同上 不同标准数值不同
        int         i_colmatrix;  // 色度矩阵设置 设置从RGB计算得到亮度和色度所用的矩阵系数 含义数值含义同上、不同标准数值不同
        int         i_chroma_loc;    // 色度样本指定 设置色度采样位置，范围0~5，默认0
    } vui;

    /* Bitstream parameters */
    int         i_frame_reference;  // 最大参考帧数目
    int         i_dpb_size;         //解码缓冲区数量
    int         i_keyint_max;        // 设定IDR帧之间的最大间隔，在此间隔设置IDR关键帧
    int         i_keyint_min;        // 设定IDR帧之间的最小间隔, 场景切换小于此值编码位I帧, 而不是 IDR帧.
    int         i_scenecut_threshold; //画面动态变化限，当超出此值时插入I帧，默认40
    int         b_intra_refresh;    // 是否使用周期帧内刷新替代IDR帧 启用后 x264编码器会在一组连续的帧序列中周期性地插入一些额外的I帧来刷新编码缓冲区。这样可以确保在视频序列中有一些I帧可以被及时解码，从而减少B帧引入的延迟，并提高视频的实时性。

    int         i_bframe;    // 两个参考帧之间的B帧数目
    int         i_bframe_adaptive; // 自适应B帧判定, 可选取值：X264_B_ADAPT_FAST等
    int         i_bframe_bias; // 控制B帧替代P帧的概率，范围-100 ~ +100，该值越高越容易插入B帧，默认0.
    int         i_bframe_pyramid;   // 允许部分B帧为参考帧， 0=off, 1=strict hierarchical, 2=normal 
    int         b_open_gop; //是否使用open gop
    int         b_bluray_compat; // 支持蓝光碟
    int         i_avcintra_class;
    /* 去块滤波器需要的参数, alpha和beta是去块滤波器参数 */
    int         b_deblocking_filter; // 去块滤波开关
    int         i_deblocking_filter_alphac0;    /* [-6, 6] -6 light filter, 6 strong */
    int         i_deblocking_filter_beta;       /* [-6, 6]  idem */

    int         b_cabac;  // 自适应算术编码cabac开关
    int         i_cabac_init_idc;  // 给出算术编码初始化时表格的选择

    int         b_interlaced; // 隔行扫描
    int         b_constrained_intra; // 强迫进行帧内预测
    /* 量化 */
    int         i_cqm_preset; // 自定义量化矩阵(CQM), 初始化量化模式为flat
    char        *psz_cqm_file; //读取JM格式的外部量化矩阵文件，忽略其他cqm选项
    //i_cqm_preset=X264_CQM_CUSTOM时候使用下面的量化矩阵
    uint8_t     cqm_4iy[16];        /* used only if i_cqm_preset == X264_CQM_CUSTOM */
    uint8_t     cqm_4py[16];
    uint8_t     cqm_4ic[16];
    uint8_t     cqm_4pc[16];
    uint8_t     cqm_8iy[64];
    uint8_t     cqm_8py[64];
    uint8_t     cqm_8ic[64];
    uint8_t     cqm_8pc[64];

    /* 日志 */
    void        (*pf_log)( void *, int i_level, const char *psz, va_list );  // 日志函数
    void        *p_log_private;//私有参数
    int         i_log_level; //日志级别
    /*重构*/
    /*
    在x264编码器中，"重构"（reconstruction）是指在编码过程中，对视频帧进行解码并恢复原始的像素数据。这是为了在编码过程中能够预测编码效果、检测编码质量等而进行的步骤。
    `b_full_recon` 参数：是一个布尔（boolean）参数。当设置为 `true`（1）时，表示要对所有编码过程中的帧进行完整的重构，这包括了编码前的参考帧（例如，前向预测的参考帧）和编码后的参考帧（例如，当前帧用于后向预测的参考帧）。这样，可以在编码过程中对视频帧的质量和效果进行实时的监测。
                                                设置为 `false`（0）时，只对编码后的参考帧进行重构。这在某些情况下可能会减少运算量和内存占用，但也会导致一些编码过程中的参考帧无法被预览。
    `psz_dump_yuv` 参数：设置输出重构帧的路径。当 `b_full_recon` 设置为 `true` 时，编码器会将编码过程中的所有重构帧输出为原始的YUV格式文件，保存在 `psz_dump_yuv` 指定的路径下。这样用户可以查看、分析和比较重构后的帧与原始帧之间的差异，以评估编码质量。
    `b_full_recon` 和 `psz_dump_yuv` 参数都是为了方便编码过程中的预览和质量评估而设定的。通过设置合适的参数，用户可以在编码过程中实时查看编码结果，以便进行调优和优化。
    */
    int         b_full_recon;   /* fully reconstruct frames, even when not necessary for encoding.  Implied by psz_dump_yuv */
    char        *psz_dump_yuv;  /* filename (in UTF-8) for reconstructed frames */

    /* 编码分析参数 */
    /*
    编码过程中的一些分析和优化参数。下面是对每个成员变量的解释：
    1. `intra`: 设置帧内（I帧）的分区数，用于分割块并进行帧内预测。
    2. `inter`: 设置帧间（P帧和B帧）的分区数，用于分割块并进行帧间预测。
    3. `b_transform_8x8`: 是否启用8x8变换，用于将8x8的块进行DCT变换。
    4. `i_weighted_pred`: P帧加权预测，用于加权预测值和参考帧之间的差异。
    5. `b_weighted_bipred`: 是否启用B帧的隐式加权预测。
    6. `i_direct_mv_pred`: 空间和时间运动矢量（MV）预测的选择。
    7. `i_chroma_qp_offset`: 色度通道的量化参数偏移值。
    8. `i_me_method`: 运动估计算法的选择（X264_ME_*）。
    9. `i_me_range`: 整数像素运动估计搜索范围（从预测的MV计算）。
    10. `i_mv_range`: MV的最大长度（以像素为单位）。-1 = 自动，根据编码级别确定。
    11. `i_mv_range_thread`: 线程之间MV的最小间隔。-1 = 自动，根据线程数确定。
    12. `i_subpel_refine`: 亚像素运动估计质量。
    13. `b_chroma_me`: P帧的色度运动估计。
    14. `b_mixed_references`: 允许每个宏块分区使用自己的参考帧编号。
    15. `i_trellis`: Trellis RD量化。
    16. `b_fast_pskip`: P帧的早期SKIP检测。
    17. `b_dct_decimate`: P帧上的变换系数阈值。
    18. `i_noise_reduction`: 自适应伪死区（pseudo-deadzone）。
    19. `f_psy_rd`: Psy RD强度。
    20. `f_psy_trellis`: Psy trellis强度。
    21. `b_psy`: 开启所有psy优化。
    22. `b_mb_info`: 在x264_picture_t中使用输入的mb_info数据。
    23. `b_mb_info_update`: 根据编码结果更新mb_info中的值。
    24. `i_luma_deadzone`: 用于亮度（Y）量化的死区大小，分别针对帧间和帧内。
    25. `b_psnr`: 是否计算和打印PSNR统计数据。
    26. `b_ssim`: 是否计算和打印SSIM统计数据。
    */
    struct
    {
        unsigned int intra;     /* intra partitions */
        unsigned int inter;     /* inter partitions */

        int          b_transform_8x8;
        int          i_weighted_pred; /* weighting for P-frames */
        int          b_weighted_bipred; /* implicit weighting for B-frames */
        int          i_direct_mv_pred; /* spatial vs temporal mv prediction */
        int          i_chroma_qp_offset;

        int          i_me_method; /* motion estimation algorithm to use (X264_ME_*) */
        int          i_me_range; /* integer pixel motion estimation search range (from predicted mv) */
        int          i_mv_range; /* maximum length of a mv (in pixels). -1 = auto, based on level */
        int          i_mv_range_thread; /* minimum space between threads. -1 = auto, based on number of threads. */
        int          i_subpel_refine; /* subpixel motion estimation quality */
        int          b_chroma_me; /* chroma ME for subpel and mode decision in P-frames */
        int          b_mixed_references; /* allow each mb partition to have its own reference number */
        int          i_trellis;  /* trellis RD quantization */
        int          b_fast_pskip; /* early SKIP detection on P-frames */
        int          b_dct_decimate; /* transform coefficient thresholding on P-frames */
        int          i_noise_reduction; /* adaptive pseudo-deadzone */
        float        f_psy_rd; /* Psy RD strength */
        float        f_psy_trellis; /* Psy trellis strength */
        int          b_psy; /* Toggle all psy optimizations */

        int          b_mb_info;            /* Use input mb_info data in x264_picture_t */
        int          b_mb_info_update; /* Update the values in mb_info according to the results of encoding. */

        /* the deadzone size that will be used in luma quantization */
        int          i_luma_deadzone[2]; /* {inter, intra} */

        int          b_psnr;    /* compute and print PSNR stats */
        int          b_ssim;    /* compute and print SSIM stats */
    } analyse;

    /* 码率控制参数 */
    /*
    设置视频编码过程中的率控（Rate Control）相关参数
    1. `i_rc_method`: 设置视频的率控方法（X264_RC_*）
    2. `i_qp_constant`: 设置恒定的量化参数（QP）值。0表示无损编码，其他值表示有损编码。
    3. `i_qp_min`: 设置允许的最小QP值。
    4. `i_qp_max`: 设置允许的最大QP值。
    5. `i_qp_step`: 设置连续帧之间的最大QP步长。
    6. `i_bitrate`: 设置目标比特率（仅在CBR和VBR模式下使用）。
    7. `f_rf_constant`: 在一遍VBR模式下的固定质量参数。 数值越小质量越好
    8. `f_rf_constant_max`: 在CRF模式下，因VBV引起的最大CRF。
    9. `f_rate_tolerance`: 率控允许的码率容差。  //平均码率的模式下，瞬时码率可以偏离的倍数，范围是0.1 ~100,默认是1.0
    10. `i_vbv_max_bitrate`: VBV的最大比特率。VBV是缓冲区控制技术 //平均码率模式下的，最大瞬时码率
    11. `i_vbv_buffer_size`: VBV的缓冲区大小。
    12. `f_vbv_buffer_init`: VBV缓冲区初始值，可以是缓冲区大小的比例或固定的比特率值。设置码率控制缓冲区（VBV）缓冲达到多满(百分比)，才开始回放，范围0~1.0，默认0.9
    13. `f_ip_factor`: I帧和P帧的比特率调节因子。  I帧和P帧之间的量化因子（QP）比值，默认1.4
    14. `f_pb_factor`: P帧和B帧的比特率调节因子。   P帧和B帧之间的量化因子（QP）比值，默认1.3
    15. `b_filler`: 是否启用VBV填充，用于强制CBR VBV并使用填充字节以确保硬件CBR。
    16. `i_aq_mode`: 自适应量化模式（X264_AQ_*），用于动态调整QP值以提高编码效率。
    17. `f_aq_strength`: 自适应量化的强度参数。
    18. `b_mb_tree`: 宏块树率控开关，用于优化码率控制。 是否开启基于macroblock的qp控制方法
    19. `i_lookahead`: 预测帧数，用于二次编码过程。 决定mbtree向前预测的帧数
    // 2pass 
    20. `b_stat_write`: 是否启用二次编码过程的统计写入。 是否将统计数据写入到文件psz_stat_out中
    21. `psz_stat_out`: 二次编码过程的统计文件的输出文件名。
    22. `b_stat_read`: 是否从文件中读取二次编码过程的统计数据。 是否从文件psz_stat_in中读入统计数据
    23. `psz_stat_in`: 二次编码过程的统计文件的输入文件名。输入文件存有第一次编码的统计数据
    //2pass params (same as ffmpeg ones)
    24. `f_qcompress`: 二次编码过程的Q压缩参数。
    25. `f_qblur`: 二次编码过程的量化模糊参数。
    26. `f_complexity_blur`: 二次编码过程的复杂度模糊参数。
    27. `zones`: 率控的区域参数，用于设置特定区域的率控参数。
    28. `i_zones`: 率控区域的数量。
    29. `psz_zones`: 率控区域的字符串表示。
    */
    struct
    {
        int         i_rc_method;    /* X264_RC_* */

        int         i_qp_constant;  /* 0 to (51 + 6*(x264_bit_depth-8)). 0=lossless */
        int         i_qp_min;       /* min allowed QP value */
        int         i_qp_max;       /* max allowed QP value */
        int         i_qp_step;      /* max QP step between frames */

        int         i_bitrate;
        float       f_rf_constant;  /* 1pass VBR, nominal QP */ //这里注释应该有问题，f_rf_constant是用于控制CRF的
        float       f_rf_constant_max;  /* In CRF mode, maximum CRF as caused by VBV */
        float       f_rate_tolerance;
        int         i_vbv_max_bitrate;
        int         i_vbv_buffer_size;
        float       f_vbv_buffer_init; /* <=1: fraction of buffer_size. >1: kbit */
        float       f_ip_factor;
        float       f_pb_factor;

        /* VBV filler: force CBR VBV and use filler bytes to ensure hard-CBR.
         * Implied by NAL-HRD CBR. */
        int         b_filler;

        int         i_aq_mode;      /* psy adaptive QP. (X264_AQ_*) */
        float       f_aq_strength;
        int         b_mb_tree;      /* Macroblock-tree ratecontrol. */
        int         i_lookahead;

        /* 2pass */
        int         b_stat_write;   /* Enable stat writing in psz_stat_out */
        char        *psz_stat_out;  /* output filename (in UTF-8) of the 2pass stats file */
        int         b_stat_read;    /* Read stat from psz_stat_in and use it */
        char        *psz_stat_in;   /* input filename (in UTF-8) of the 2pass stats file */

        /* 2pass params (same as ffmpeg ones) */
        float       f_qcompress;    /* 0.0 => cbr, 1.0 => constant qp */
        float       f_qblur;        /* temporally blur quants */
        float       f_complexity_blur; /* temporally blur complexity */
        x264_zone_t *zones;         /* ratecontrol overrides */
        int         i_zones;        /* number of zone_t's */
        char        *psz_zones;     /* alternate method of specifying zones */
    } rc;

    /*裁剪区域，只对裁剪的区域进行编码*/
    struct
    {
        unsigned int i_left;
        unsigned int i_top;
        unsigned int i_right;
        unsigned int i_bottom;
    } crop_rect;

    /* frame packing arrangement flag */
    int i_frame_packing;//编码3D视频时，此参数在码流中插入一个标志，告知解码器此3D视频是如何封装的。3D视频帧封装格式：帧封装、场交错、线交错、左右全景、左右半景、上下半景、L+深度、L+深度+图形+图形深度

    /*h264复用参数*/
    int b_aud;                  // 生成访问单元分隔符
    int b_repeat_headers;       // 是否复制sps和pps放在每个关键帧的前面
    int b_annexb;               // 值为true，则NALU之前是4字节前缀码0x00000001；值为false，则NALU之前的4个字节为NALU长度
    int i_sps_id;               // sps和pps的id号
    int b_vfr_input;             // VFR输入。1 ：时间基和时间戳用于码率控制  0 ：仅帧率用于码率控制
    int b_pulldown;             /* use explicity set timebase for CFR */
    uint32_t i_fps_num;  //帧率分子
    uint32_t i_fps_den; //帧率分母
    uint32_t i_timebase_num;     // 时间基的分子
    uint32_t i_timebase_den;     // 时间基的分母

    int b_tff;
    int b_pic_struct;
    int b_fake_interlaced;// 将视频流标记为交错(隔行)，哪怕并非为交错式编码。可用于编码蓝光兼容的25p和30p视频。默认是未开启 
    int b_stitchable;

    int b_opencl;            /* use OpenCL when available */
    int i_opencl_device;     /* specify count of GPU devices to skip, for CLI users */
    void *opencl_device_id;  /* pass explicit cl_device_id as void*, for API users */
    char *psz_clbin_file;    /* filename (in UTF-8) of the compiled OpenCL kernel cache file */

    /* 条带参数 */
    int i_slice_max_size;    // 每个slice的最大字节数，包括预计的NAL开销
    int i_slice_max_mbs;     // 每个slice的最大宏块数，重写i_slice_count
    int i_slice_min_mbs;     //每隔slice最小条带数
    int i_slice_count;       // 每帧slice的数目，每个slice必须是矩形
    int i_slice_count_max;   /*设置每帧最大切片数量，设置这个i_slice_max_size和i_slice_max_mbs就会失效 Absolute cap on slices per frame; stops applying slice-max-size
                              * and slice-max-mbs if this is reached. */

    void (*param_free)( void* );
    void (*nalu_process)( x264_t *h, x264_nal_t *nal, void *opaque );//编码器生成一个NALU的时候回调该函数
} x264_param_t;

