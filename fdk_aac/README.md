音频压缩的方式有很多种：

在一个样本内部进行压缩。
相邻两个样本间取不相同的地方（做差）进行压缩，所以在播放时，后一个样本依赖于前一个样本。
将样本以块来处理，取平均值。
g711：帧内压缩，压缩过程不依赖于前后帧的信息 视频才有帧间压缩
    *在样本内部进行压缩。将一个13bit或14bit的样本编码成一个8bit的样本。g711标准主要分两种压缩方法：a-law和mu-law，
    *3bit14bit取决于你A/D转换器的量化精度，对13bit精度的量化值进行编码，得到一个13bit二进制pcm样本值，也得用2字节也就是16bit去存在这种编码规则下
     13位14位和16位的样本都是用2字节来存，无法将他们区分出来，所以就看你是让高13位赋予意义还是让低13位赋予意义。所有G711也就是把16位数据压缩成8位，所有就要求原始的PCM是16bits的，不然无法用G711压缩
    *a-law：将一个13bit的pcm样本压缩成一个8bit的pcm样本。在16bit的pcm中挑出13bit，经过了13折线法，生成了一个8bit的样本，压缩后的数据只有8bit，用char来存放
    *u-law：将一个14bit的pcm样本压缩成一个8bit的pcm样本。同上
    *g711一帧有160个采样点，压缩后的PCMA PCMU也就是有160个字节
    *g711是有损压缩
    *通常，G.711在VoIP（Voice over Internet Protocol）等应用中使用，VoIP中的音频采样率通常是8000 Hz，因为这个采样率对于电话通话已经足够。此时的比特率是64kbps
     16 kHz、44.1 kHz或48 kHz等，也可以使用G.711进行压缩，只要将相应的采样率的PCM音频样本转换为16位并应用G.711压缩算法即可。在解码端，再将压缩后的G.711音频解压缩为PCM，并根据相应的采样率进行播放。

AAC：帧内压缩
    *AAC：帧大小1024个sample MP3：帧大小1152个sample
    *AAC对PCM位深没有要求，不过实际中音频一般都是>=16bits
    https://blog.csdn.net/weixin_41910694/article/details/107735932
    1. MPEG-2 AAC LC 低复杂度规格（Low Complexity），编码方式比较简单，没有增益控制，但是提高了编码效率，在中等码率的编码效率和音质方面，都能找到平衡点；
    2. MPEG-2 AAC Main 主规格
    3. MPEG-2 AAC SSR 可变采样率规格（Scaleable Sample Rate）
    4. MPEG-4 AAC LC 低复杂度规格（Low Complexity）
    5. MPEG-4 AAC Main 主规格，包含了除增益控制之外的全部功能，音质最好
    6. MPEG-4 AAC SSR 可变采样率规格（Scaleable Sample Rate）
    7. MPEG-4 AAC LTP 长时期预测规格（Long Term Prediction）
    8. MPEG-4 AAC LD 低延迟预测规格（Low Delay）
    9. MPEG-4 AAC HE 高效率规格（High Efficency），这种规格适合用于低码率编码，有 Nero-ACC 编码器支持，是一种成熟的商用编码器；
    最常用常用的规格如下：
        LC-AAC就是比较传统的AAC，主要用于中高码率(>=80Kbps)
        HE-AAC(相当于AAC+SBR)，主要用于中低码(<=80Kbps)
        新近推出的HE-AACv2(相当于AAC+SBR+PS)，主要用于低码率(<=48Kbps)
# fdk_aac解码

# fdk_aac 编码
typedef enum {
  AACENC_AOT = 0x0100, /*!< 音频对象类型。详见 FDK_audio.h 中的 ::AUDIO_OBJECT_TYPE*/
  AACENC_BITRATE = 0x0101, /*!< 编码器的总比特率。此参数是强制性的，并与 ::AACENC_BITRATEMODE 交互。
                              - CBR：固定比特率，以比特/秒为单位。
                              - VBR：可变比特率。忽略比特率参数*/
  AACENC_BITRATEMODE = 0x0102, /*!< 比特率模式。可以配置不同类型的比特率配置：
                                  - 0: 固定比特率，使用 ::AACENC_BITRATE 中指定的比特率。
                                  - 1: 可变比特率模式，\ref vbrmode "非常低比特率"。
                                  - 2: 可变比特率模式，\ref vbrmode "低比特率"。
                                  - 3: 可变比特率模式，\ref vbrmode "中比特率"。
                                  - 4: 可变比特率模式，\ref vbrmode "高比特率"。
                                  - 5: 可变比特率模式，\ref vbrmode "非常高比特率"。*/
  AACENC_SAMPLERATE = 0x0103, /*!< 采样率：8000、11025、12000、16000、22050、24000、32000、44100、48000、64000、88200、96000。*/
  AACENC_SBR_MODE = 0x0104, /*!< 独立配置 SBR(音频编码技术，在低比特率下提供更高的频带扩展实现更好的音频质量，SBR 常用于 HE-AAC)，与所选的音频对象类型 ::AUDIO_OBJECT_TYPE 无关。此参数仅适用于 ELD 音频对象类型(AUDIO_OBJECT_TYPE::AOT_ER_AAC_ELD)。
                               - -1: 使用 ELD SBR 自动配置器（默认值）。
                               - 0: 禁用 Spectral Band Replication。
                               - 1: 启用 Spectral Band Replication。*/
  AACENC_GRANULE_LENGTH = 0x0105, /*!< 核心编码器（AAC）的音频帧长度，以样本数为单位：
                                     - 1024: 默认配置。
                                     - 512: 在 LD/ELD 配置中的默认长度。
                                     - 480: 在 LD/ELD 配置中的长度。
                                     - 256: ELD 减少延迟模式的长度（x2）。
                                     - 240: ELD 减少延迟模式的长度（x2）。
                                     - 128: ELD 减少延迟模式的长度（x4）。
                                     - 120: ELD 减少延迟模式的长度（x4）。*/
  AACENC_CHANNELMODE = 0x0106, /*!< 设置显式的声道模式。声道模式必须与输入通道数相匹配。FDK_audio.h 中的 ::CHANNEL_MODE。*/
  AACENC_CHANNELORDER = 0x0107, /*!< 输入音频数据的声道排序方案：
                                   - 0: MPEG 声道排序（例如 5.1: C, L, R, SL, SR, LFE）。 （默认值）
                                   - 1: WAVE 文件格式的声道排序（例如 5.1: L, R, C, LFE, SL, SR）。*/
  AACENC_SBR_RATIO = 0x0108, /*!< 控制降采样 SBR 的激活。降采样 SBR 支持 AAC-ELD 和 HE-AACv1。
                                - 1: 降采样 SBR（默认值，用于 ELD）。
                                - 2: 双速率 SBR（默认值，用于 HE-AAC）。*/
  AACENC_AFTERBURNER = 0x0200, /*!< 此参数控制使用 afterburner 功能。afterburner 是一种分析与合成算法，可以提高音频质量，但也会增加所需的处理能力。建议在不影响附加内存消耗和处理能力的情况下始终激活此功能。
                                  - 0: 禁用 afterburner（默认值）。
                                  - 1: 启用 afterburner。*/
  AACENC_BANDWIDTH = 0x0203, /*!< 核心编码器音频带宽：
                                - 0: 内部确定音频带宽（默认值，详见章节 \ref BEHAVIOUR_BANDWIDTH）。
                                - 1 到 fs/2: 音频带宽（赫兹）。最大限制为 20kHz。如果 SBR 处于活动状态，则不可用。此设置仅供专家使用，请勿更改此值以避免降低音频质量。*/
  AACENC_PEAK_BITRATE = 0x0207, /*!< 峰值比特率配置参数，用于调整每个音频帧的最大比特数。比特率以比特/秒为单位。峰值比特率在内部将限制为所选比特率 ::AACENC_BITRATE 的下限和 number_of_effective_channels*6144 比特的上限。
                                  将峰值比特率设置为 ::AACENC_BITRATE 并不一定意味着音频帧的大小是恒定的。由于峰值比特率是以比特/秒为单位，因此帧大小可能在一个或另一个方向上在各个帧之间变化一个字节。然而，不建议将峰值比特率减小到 ::AACENC_BITRATE - 这将禁用比特保留，从而严重影响音频质量。*/
  AACENC_TRANSMUX = 0x0300, /*!< 要使用的传输类型。详见 FDK_audio.h 中的 ::TRANSPORT_TYPE。编码器库中可以配置以下类型：
                               - 0: 原始访问单元
                               - 1: ADIF 比特流格式
                               - 2: ADTS 比特流格式
                               - 6: 包含 muxConfigPresent=1 的音频多路复用元素（LATM）
                               - 7: 包含 muxConfigPresent=0 的音频多路复用元素（LATM），带外 StreamMuxConfig
                               - 10: 音频同步流（LOAS）*/
  AACENC_HEADER_PERIOD = 0x0301, /*!< 在 LATM/LOAS 传输层中发送帧计数周期内的 in-band 配置缓冲区。此外，此参数还配置 raw_data_block() 中的 PCE 重复周期。详见 \ref encPCE。
                                    - 0xFF: 自动模式，默认值为 10，用于 TT_MP4_ADTS、TT_MP4_LOAS 和 TT_MP4_LATM_MCP1，否则为 0。
                                    - n: 帧计数周期。*/
  AACENC_SIGNALING_MODE = 0x0302, /*!< 扩展 AOT 的信号模式：
                                    - 0: 隐式向后兼容信号（非 MPEG-4 基于 AOT 和传输格式 ADIF、ADTS 的默认值）
                                          - 使用隐式信号的流可以被任何 AAC 解码器解码，即使是 AAC-LC 专用解码器。
                                          - 仅会解码流的低频部分，导致带限输出。
                                          - 此方法适用于所有传输格式。
                                          - 此方法不适用于降采样 SBR。
                                    - 1: 显式向后兼容信号
                                          - 使用显式向后兼容信号的流可以被任何 AAC 解码器解码，即使是 AAC-LC 专用解码器。
                                          - 仅会解码流的低频部分，导致带限输出。
                                          - 不支持 ADIF 或 ADTS。对于 LOAS/LATM，仅在 AudioMuxVersion==1 下工作。
                                          - 此方法适用于降采样 SBR。
                                    - 2: 显式分层信号（MPEG-4 基于 AOT 和所有传输格式除外 ADIF 和 ADTS 的默认值）
                                          - 仅有 HE-AAC 解码器可以解码使用显式分层信号的流。
                                          - AAC-LC 专用解码器将无法解码使用显式分层信号的流。
                                          - 未解码 PS 的解码器在 PS 存在的情况下将只解码 AAC-LC+SBR 部分。如果流包含 PS，则结果将是解码的单声道混音。
                                          - 不支持 ADIF 或 ADTS。对于 LOAS/LATM，仅在 AudioMuxVersion==1 下工作。
                                          - 此方法适用于降采样 SBR。
                                    为了确保听者始终获得最佳音频质量，应使用显式分层信号。这将确保只有完全支持 HE-AAC 的解码器才会解码这些流。音频以完整带宽播放。为了最佳向后兼容性，建议使用隐式 SBR 信号进行编码。AAC-LC 专用解码器将只解码 AAC 部分，这意味着解码的音频将会带限。
                                    对于 MPEG-2 传输类型（ADTS、ADIF），仅允许使用隐式信号。
                                    对于 LOAS 和 LATM，显式向后兼容信号仅与 AudioMuxVersion==1 一起工作。原因是，对于显式向后兼容信号，将附加信息追加到 ASC 中。仅支持 AAC-LC 的解码器将跳过此部分。然而，为了跳转到 ASC 的末尾，它需要知道 ASC 的长度。传输 ASC 的长度是 AudioMuxVersion==1 的特性，不可能使用 AudioMuxVersion==0 传输 ASC 的长度，因此只能用 AudioMuxVersion==0 编码的 LOAS/LATM 流解析 AAC-LC 专用解码器将无法解析。
                                    对于降采样 SBR，显式信号是强制性的。原因是扩展采样频率（在 SBR 的情况下是 SBR 部分的采样频率）只能在显式模式下进行信号化。
                                    对于 AAC-ELD，SBR 信息传输在 ELDSpecific Config 中，它是 AudioSpecificConfig 的一部分。因此，这里的设置对 AAC-ELD 没有影响。*/
  AACENC_TPSUBFRAMES = 0x0303, /*!< LOAS/LATM 或 ADTS 中传输帧的子帧数（默认值为 1）。
                                   - ADTS: 最大子帧数限制为 4。
                                   - LOAS/LATM: 最大子帧数限制为 2。*/
  AACENC_AUDIOMUXVER = 0x0304, /*!< 用于 LATM 的 AudioMuxVersion（AudioMuxVersionA，目前未实现）：
                                   - 0: 默认值，不传输 tara Buffer fullness，不传输 ASC 长度，包括实际的 latm Buffer fullness。
                                   - 1: 传输 tara Buffer fullness，ASC 长度和实际的 latm Buffer fullness。
                                   - 2: 传输 tara Buffer fullness，ASC 长度和 latm Buffer fullness 的最大值。*/
  AACENC_PROTECTION = 0x0306, /*!< 配置传输层中的保护：
                                 - 0: 无保护。（默认值）
                                 - 1: ADTS 传输格式的 CRC 活动。*/
  AACENC_ANCILLARY_BITRATE = 0x0500, /*!< 常量辅助数据比特率，以比特/秒为单位。
                                        - 0: 要么没有辅助数据，要么插入由输入参数 numAncBytes 指定的确切字节数，详见 AACENC_InArgs。
                                        - 其他: 使用指定的比特率插入辅助数据。*/
  AACENC_METADATA_MODE = 0x0600, /*!< 配置元数据。
                                    - 0: 不嵌入任何元数据。
                                    - 1: 嵌入 dynamic_range_info 元数据。
                                    - 2: 嵌入 dynamic_range_info 和 ancillary_data 元数据。
                                    - 3: 嵌入 ancillary_data 元数据。*/
  AACENC_CONTROL_STATE = 0xFF00, /*!< 存在一个自动进程，当配置参数更改或发生错误时，内部重新配置编码器实例。此参数允许覆盖或获取此过程的控制状态。详见 ::AACENC_CTRLFLAGS。*/
  AACENC_NONE = 0xFFFF /*!< ------ */
} AACENC_PARAM;



-rw-r--r-- 1 13919 197609  1764000 Mar  3 21:34 audio.pcm

$ ./ffprobe -ar 44100 -ac 2 -f s16le -i audio.pcm
Input #0, s16le, from 'audio.pcm':
  Duration: 00:00:10.00, bitrate: 1411 kb/s
  Stream #0:0: Audio: pcm_s16le, 44100 Hz, 2 channels, s16, 1411 kb/s

PCM format:
    L R L R L R L R ....
    ↓
    2Byte

|   1764000 Byte / ( 44100HZ * 2 channls * 2 Byte ) = 10秒   |



比特率：bps（bit per second，位/秒）；kbps（通俗地讲就是每秒钟1000比特）作为单位。
    比特率=采样率*采样声道数*采样位深
    1411200 b/s = 44100*2*16 即 1411.2 kb/s


 see fdk-aac project of documentation/aacEncoder.pdf from github with https://github.com/mstorsjo/fdk-aac.

    2.2 Calling Sequence

        1. Call aacEncOpen() to allocate encoder instance with required configuration.

        2. Call aacEncoder SetParam() for each parameter to be set. AOT(Audio Object Type), samplingrate, channelMode,
        bitrate and transport type are mandatory.

        3. Call aacEncEncode() with NULL parameters to initialize encoder instance with present parameter set.

        4. Call aacEncInfo() to retrieve a configuration data block to be transmitted out of band. This
        is required when using RFC3640 or RFC3016 like transport.

        5. Encode input audio data in loop.

        6. Call aacEncClose() and destroy encoder instance.


