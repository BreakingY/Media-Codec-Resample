#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__cplusplus)
extern "C" {
#include "fdk-aac/aacdecoder_lib.h"
};
#else
#include "fdk-aac/aacdecoder_lib.h"
#endif
class adts_header_t
{
public:
    unsigned char syncword_0_to_8 : 8;

    unsigned char protection_absent : 1;
    unsigned char layer : 2;
    unsigned char ID : 1;
    unsigned char syncword_9_to_12 : 4;

    unsigned char channel_configuration_0_bit : 1;
    unsigned char private_bit : 1;
    unsigned char sampling_frequency_index : 4;
    unsigned char profile : 2;

    unsigned char frame_length_0_to_1 : 2;
    unsigned char copyrignt_identification_start : 1;
    unsigned char copyright_identification_bit : 1;
    unsigned char home : 1;
    unsigned char original_or_copy : 1;
    unsigned char channel_configuration_1_to_2 : 2;

    unsigned char frame_length_2_to_9 : 8;

    unsigned char adts_buffer_fullness_0_to_4 : 5;
    unsigned char frame_length_10_to_12 : 3;

    unsigned char number_of_raw_data_blocks_in_frame : 2;
    unsigned char adts_buffer_fullness_5_to_10 : 6;
};
int main(int argc, char const *argv[])
{
    if (argc < 6) {
        printf("./bin input ouput samplerate channels bits\n");
        return -1;
    }
    FILE *inputFD = fopen(argv[1], "r");
    FILE *outputFD = fopen(argv[2], "w");
    if (inputFD == NULL || outputFD == NULL) {
        printf("fail to open file\r\n");
        return -1;
    }
    int samplerate = atoi(argv[3]);
    int channels = atoi(argv[4]);
    int bits = atoi(argv[5]);
    printf("input:%s ouput:%s samplerate:%d channels:%d bits:%d\n", argv[1], argv[2], samplerate, channels, bits);
    HANDLE_AACDECODER dec = NULL;
    dec = aacDecoder_Open(TT_MP4_ADTS, 1); // nrOfLayers表示AAC流的层数,基本上填1。
    if (!dec) {
        printf("aacDecoder_Open errror\n");
        return -1;
    }
    int nbPcm = channels * 1024 * (bits / 8) * 2; // 一帧aac解码后得长度为channels*1024*(bits/8) 乘2扩大内存

    char *pcm_buf = (char *)malloc(nbPcm);

    fseek(inputFD, 0, SEEK_END);
    int nbAacSize = ftell(inputFD);
    fseek(inputFD, 0, SEEK_SET);

    char *aac_buf = (char *)malloc(nbAacSize);
    fread(aac_buf, 1, nbAacSize, inputFD);
    int pos = 0;
    CStreamInfo *info = NULL;
    while (1) {
        if (nbAacSize - pos < 7) {
            break;
        }
        adts_header_t *adts = (adts_header_t *)(aac_buf + pos);
        if (adts->syncword_0_to_8 != 0xff || adts->syncword_9_to_12 != 0xf) {
            break;
        }
        // protection_absent=0时, header length=9bytes
        // protection_absent=1时, header length=7bytes
        // frame_length : ⼀个ADTS帧的⻓度包括ADTS头和AAC原始流.

        int aac_frame_size = adts->frame_length_0_to_1 << 11 | adts->frame_length_2_to_9 << 3 | adts->frame_length_10_to_12;
        if (pos + aac_frame_size > nbAacSize) {
            printf("pos + aac_frame_size:%d aac_frame_size:%d nbAacSize:%d\n\n", pos + aac_frame_size, aac_frame_size, nbAacSize);
            break;
        }

        unsigned char *udata = (unsigned char *)aac_buf + pos;
        unsigned int unb_data = (unsigned int)aac_frame_size;
        unsigned int unb_left = unb_data;
        AAC_DECODER_ERROR err = aacDecoder_Fill(dec, &udata, &unb_data, &unb_left);
        if (err != AAC_DEC_OK) {
            return err;
        }
        pos += aac_frame_size;
        if (unb_left > 0) {
            printf("unb_left:%d\n", unb_left);
            continue;
        }

        INT_PCM *upcm = (INT_PCM *)pcm_buf;
        int unb_pcm = (int)nbPcm;
        err = aacDecoder_DecodeFrame(dec, upcm, unb_pcm, 0);
        // user should fill more bytes then decode.
        if (err == AAC_DEC_NOT_ENOUGH_BITS) {
            continue;
        }
        if (err != AAC_DEC_OK) {
            return err;
        }

        // when decode ok, retrieve the info.
        if (!info) {
            info = aacDecoder_GetStreamInfo(dec);
        }
        // the actual size of pcm.
        int pnb_valid = channels * info->frameSize * (bits / 8);
        if (pnb_valid != channels * 1024 * (bits / 8)) {
            printf("pnb_valid:%d frameSize:%d\n", pnb_valid, info->frameSize);
        }
        fwrite((unsigned char *)&pcm_buf[0], pnb_valid, 1, outputFD);
    }
    fclose(inputFD);
    fclose(outputFD);
    aacDecoder_Close(dec);
    free(pcm_buf);
    free(aac_buf);
    return 0;
}
