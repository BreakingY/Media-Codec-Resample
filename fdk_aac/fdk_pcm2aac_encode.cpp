#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__cplusplus)
extern "C" {
#include "fdk-aac/aacenc_lib.h"
};
#else
#include "fdk-aac/aacenc_lib.h"
#endif
typedef struct parameterDict {
    AACENC_PARAM param;
    UINT value;
} Dict;
static int readMaxLen = 0;
static int samplerate;
static int channels;
static int bits;
typedef struct AacContext {
    HANDLE_AACENCODER hAacEncoder;
    int inBufferIdentifier;
    int inputbufElSizes;

    int outputbufElSizes;
    int outputIdentifier;
    int outputBufferSize;

    AACENC_BufDesc inputBufDesc;
    AACENC_BufDesc outputBufDesc;
    AACENC_InArgs inputArgs;
    AACENC_OutArgs outputArgs;
} AacCtx;

int aacEncInit(AacCtx *ctx);
int aacEncEnd(AacCtx *ctx);

int aacEncInit(AacCtx *ctx)
{
    if (!ctx)
        return -1;
    AACENC_ERROR AAC_ERR = AACENC_OK;
    Dict m_Dict[] = {
        {AACENC_AOT, AOT_AAC_LC},           // AAC-LC
        {AACENC_SBR_MODE, AOT_NULL_OBJECT}, // 禁用sbr
        {AACENC_SAMPLERATE, samplerate},
        {AACENC_BITRATEMODE, 3}, // 0：固定比特率(使用AACENC_BITRATE指定的比特率) 1-5：可变比特率模式 1: "非常低比特率" 2: "低比特率" 3: "中比特率" 4: "高比特率" 5: "非常高比特率"
        {AACENC_CHANNELMODE, channels == 2 ? MODE_2 : MODE_1},
        {AACENC_TRANSMUX, TT_MP4_ADTS}, // 带有adts
    };
    // encModules：通常默认0，表示编码器内部分配内存 maxChannels：音频通道数，范围1～8 0：分配最大通道数
    AAC_ERR = aacEncOpen(&ctx->hAacEncoder, 0x00, 0x00);
    if (ctx->hAacEncoder == NULL || AAC_ERR != AACENC_OK) {
        printf("aacEncOpen ERROR\r\n");
        return -1;
    }

    int i = 0, j = sizeof(m_Dict) / sizeof(Dict);
    for (i = 0; i < j; i++) {
        AAC_ERR = aacEncoder_SetParam(ctx->hAacEncoder, m_Dict[i].param, m_Dict[i].value);
        if (AAC_ERR != AACENC_OK) {
            aacEncEnd(ctx);
            ctx->hAacEncoder = NULL;
            printf("aacEncoder_SetParam ERROR %d\r\n", i);
            return -1;
        }
    }
    // 四个参数都是 NULL，是告诉 AAC 编码器执行编码操作，但没有提供要编码的音频数据。这实际上是一个空操作，可能是在特定情况下暂时不需要编码数据时使用的。
    if (aacEncEncode(ctx->hAacEncoder, NULL, NULL, NULL, NULL) != AACENC_OK) {
        aacEncEnd(ctx);
        ctx->hAacEncoder = NULL;
        printf("aacEncEncode test ERROR from aacEncInit\n");
        return -1;
    }
#if 0
    typedef struct {
        INT numBufs;            /*!< Number of buffers.buffer个数 */
        void **bufs;            /*!< Pointer to vector containing buffer addresses.地址 */
        INT *bufferIdentifiers; /*!< Identifier of each buffer element. See
                                    ::AACENC_BufferIdentifier. */
        INT *bufSizes;          /*!< Size of each buffer in 8-bit bytes. 每个buffer的大小*/
        INT *bufElSizes;        /*!< Size of each buffer element in bytes. 每个采样点大小*/
    } AACENC_BufDesc;
#endif
    ctx->inBufferIdentifier = IN_AUDIO_DATA;
    ctx->inputBufDesc.bufferIdentifiers = &ctx->inBufferIdentifier;
    ctx->inputbufElSizes = bits / 8;
    ctx->inputBufDesc.bufElSizes = &ctx->inputbufElSizes;

    ctx->outputIdentifier = OUT_BITSTREAM_DATA;
    ctx->outputBufDesc.bufferIdentifiers = &ctx->outputIdentifier;
    ctx->outputbufElSizes = 1;
    ctx->outputBufDesc.bufElSizes = &ctx->outputbufElSizes;

    ctx->inputBufDesc.numBufs = 1;
    ctx->outputBufDesc.numBufs = 1;
    return 0;
}
#if 0
/**
 *  Defines the input arguments for an aacEncEncode() call.
 */
typedef struct {
  INT numInSamples; /*!< 多通道样本个数总和. */
  INT numAncBytes;  /*!< 辅助数据个数，如果有辅助数据，则 inputBufDesc中的buffer应该有两个，第一个放PCM数据，第二个放辅助数据*/
} AACENC_InArgs;
int numInSamples = 1024; // 假设有 1024 个样本
int numAncBytes = 64;    // 假设有 64 字节的辅助数据

//添加辅助数
uint8_t *audioData = /* ... */;
uint8_t *ancillaryData = /* ... */;
// 填充 AACENC_InArgs 结构体
AACENC_InArgs inArgs;
inArgs.numInSamples = numInSamples;
inArgs.numAncBytes = numAncBytes;

// 创建 AACENC_BufDesc 结构体
AACENC_BufDesc bufDesc;
bufDesc.numBufs = 2; // 输入缓冲区有两个：一个用于音频数据，一个用于辅助数据
bufDesc.bufs = (void **)malloc(2 * sizeof(void *));
bufDesc.bufferIdentifiers = (INT *)malloc(2 * sizeof(INT));
bufDesc.bufSizes = (INT *)malloc(2 * sizeof(INT));
bufDesc.bufElSizes = (INT *)malloc(2 * sizeof(INT));
// 音频数据
bufDesc.bufs[0] = audioData;
bufDesc.bufferIdentifiers[0] = /* 标识音频数据 */;
bufDesc.bufSizes[0] = numInSamples * sizeof(short);
bufDesc.bufElSizes[0] = sizeof(short);
//辅助数据
bufDesc.bufs[1] = ancillaryData;
bufDesc.bufferIdentifiers[1] = /* 标识辅助数据 */;
bufDesc.bufSizes[1] = numAncBytes;
bufDesc.bufElSizes[1] = sizeof(uint8_t);
// 调用 aacEncEncode() 函数，传递输入参数
aacEncEncode(encoderHandle, &bufDesc, NULL, &inArgs, NULL);

/**
 *  Defines the output arguments for an aacEncEncode() call.
 */
typedef struct {
  INT numOutBytes;  /*!< aac数据长度 */
  INT numInSamples; /*!< 同AACENC_InArgs*/
  INT numAncBytes;  /*!< 同AACENC_InArgs*/
  INT bitResState;  /*!< State of the bit reservoir in bits. */

} AACENC_OutArgs;
#endif
int aacEncode(AacCtx *ctx, void *pPCMdata, int PCMdataSize, void *outputBuffer, int outputBufferSize)
{
    /* input buffer info */
    ctx->inputBufDesc.bufs = &pPCMdata;
    ctx->inputBufDesc.bufSizes = &PCMdataSize;

    /* output buffer info */
    ctx->outputBufDesc.bufs = &outputBuffer;
    ctx->outputBufferSize = outputBufferSize;
    ctx->outputBufDesc.bufSizes = (INT *)&ctx->outputBufferSize;

    /* input arguments */
    ctx->inputArgs.numInSamples = PCMdataSize / (bits / 8); // 两个通道加起来的样本个数

    /* encode */
    if (aacEncEncode(ctx->hAacEncoder, &ctx->inputBufDesc, &ctx->outputBufDesc, &ctx->inputArgs, &ctx->outputArgs) != AACENC_OK) {
        printf("aacEncEncode AACENC CODE %d, output bytes : %d\r\n", ctx->outputArgs.numOutBytes);
        return 0;
    }
    return ctx->outputArgs.numOutBytes;
}

int aacEncEnd(AacCtx *ctx)
{
    if (!ctx || !ctx->hAacEncoder) {
        return -1;
    }

    if (aacEncClose(&ctx->hAacEncoder) != AACENC_OK) {
        return -1;
    }

    ctx->hAacEncoder = NULL;
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 6) {
        printf("./bin input ouput samplerate channels bits\n");
        return -1;
    }
    samplerate = atoi(argv[3]);
    channels = atoi(argv[4]);
    bits = atoi(argv[5]);
    readMaxLen = 1024 * channels * (bits / 8);

    AacCtx ctx = {NULL};
    if (aacEncInit(&ctx) != 0 || !ctx.hAacEncoder) {
        printf("init Fail!\r\n");
        return -1;
    }
    // aac一帧1024个采样点  readMaxLen=1024*channel*bites/8=1024*2*2=4096
    unsigned char inputBuffer[readMaxLen] = {0};
    unsigned char outputBuffer[readMaxLen] = {0};

    FILE *inputFD = fopen(argv[1], "r");
    FILE *outputFD = fopen(argv[2], "w");
    if (inputFD == NULL || outputFD == NULL) {
        printf("fail to open file\r\n");
        return -1;
    }

    size_t readLen = 0;
    do {
        readLen = fread(inputBuffer, 1, readMaxLen, inputFD);
        int ret = aacEncode(&ctx, inputBuffer, readLen, outputBuffer, readMaxLen);
        printf("aac size:%d\n", ret);
        fwrite(outputBuffer, ret, 1, outputFD);
    } while (readLen > 0);

    fclose(inputFD);
    fclose(outputFD);

    aacEncEnd(&ctx);
    return 0;
}
