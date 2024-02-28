#include <stdio.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#include "x265.h"
};
#else
#include "x265.h"
#endif
int main(int argc, char **argv)
{
    if (argc < 6) {
        printf("./bin input ouput width height fps\n");
        return 1;
    }
    int i, j;
    FILE *fp_src = NULL;
    FILE *fp_dst = NULL;
    int y_size;
    int buff_size;
    char *buff = NULL;
    int ret;
    x265_nal *pNals = NULL;
    uint32_t iNal = 0;

    x265_param *pParam = NULL;
    x265_encoder *pHandle = NULL;
    x265_picture *pPic_in = NULL;

    int frame_num = 0;
    int csp = X265_CSP_I420;
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);

    fp_src = fopen(argv[1], "rb");

    fp_dst = fopen(argv[2], "wb");
    // Check
    if (fp_src == NULL || fp_dst == NULL) {
        return -1;
    }

    pParam = x265_param_alloc();
    x265_param_default(pParam);
    x265_param_default_preset(pParam, "fast", "zerolatency"); // 设置preset和tune

    pParam->bRepeatHeaders = 1; // write sps,pps before keyframe
    pParam->bAnnexB = 1;
    pParam->internalCsp = csp;
    pParam->sourceWidth = width;
    pParam->sourceHeight = height;
    pParam->fpsNum = atoi(argv[5]);
    pParam->fpsDenom = 1;

    pParam->bframes = 0;
    pParam->keyframeMin = 50;
    pParam->keyframeMax = 50;

    // 码率控制
    pParam->rc.rateControlMode = X265_RC_ABR;
    pParam->rc.bitrate = 2048;
    // x265码率控制必须把vbvMaxBitrate  vbvBufferSize进行赋值,否则码率控制失效
    pParam->rc.vbvMaxBitrate = 2048;
    pParam->rc.vbvBufferSize = 2048;

    // set profile
    x265_param_apply_profile(pParam, "main");
    // Init
    pHandle = x265_encoder_open(pParam);
    if (pHandle == NULL) {
        printf("x265_encoder_open err\n");
        return 0;
    }
    y_size = pParam->sourceWidth * pParam->sourceHeight;

    pPic_in = x265_picture_alloc();
    x265_picture_init(pParam, pPic_in);
    switch (csp) {
    case X265_CSP_I444: {
        buff = (char *)malloc(y_size * 3);
        pPic_in->planes[0] = buff;
        pPic_in->planes[1] = buff + y_size;
        pPic_in->planes[2] = buff + y_size * 2;
        pPic_in->stride[0] = width;
        pPic_in->stride[1] = width;
        pPic_in->stride[2] = width;
        break;
    }
    case X265_CSP_I420: {
        buff = (char *)malloc(y_size * 3 / 2);
        pPic_in->planes[0] = buff;                  // 0
        pPic_in->planes[1] = buff + y_size;         // 1->U->1 + 1/4 = 5/4
        pPic_in->planes[2] = buff + y_size * 5 / 4; // 1.25->V->5/4 + 1/4 = 3/2
        /*
                Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y
                Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y
                Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y
                Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y      Y Y Y Y Y Y
                U U U U U U      V V V V V V      U V U V U V      V U V U V U
                V V V V V V      U U U U U U      U V U V U V      V U V U V U
                - I420 -          - YV12 -         - NV12 -         - NV21 -

        */
        pPic_in->stride[0] = width;
        // I420下一张图像下 U V的像素个数都分别是y_size(w*h)/4，但stride是一行的数据量，不是总数据量
        /*
            YUV420在packet下是
            Y		Y		Y		Y		Y		Y		Y		Y
                UV				UV				UV				UV
            Y		Y		Y		Y		Y		Y		Y		Y
            Y		Y		Y		Y		Y		Y		Y		Y
                UV				UV				UV				UV
            Y		Y		Y		Y		Y		Y		Y		Y
        这里面的stride应该是上面的排列方式说明，所以 U V的stride是width / 2
        */
        pPic_in->stride[1] = width / 2;
        pPic_in->stride[2] = width / 2;
        break;
    }
    default: {
        printf("Colorspace Not Support.\n");
        return -1;
    }
    }

    // detect frame number
    fseek(fp_src, 0, SEEK_END);
    switch (csp) {
    case X265_CSP_I444:
        frame_num = ftell(fp_src) / (y_size * 3);
        break;
    case X265_CSP_I420:
        frame_num = ftell(fp_src) / (y_size * 3 / 2);
        break;
    default:
        printf("Colorspace Not Support.\n");
        return -1;
    }
    fseek(fp_src, 0, SEEK_SET);

    // Loop to Encode
    for (i = 0; i < frame_num; i++) {
        switch (csp) {
        case X265_CSP_I444: {
            fread(pPic_in->planes[0], 1, y_size, fp_src); // Y
            fread(pPic_in->planes[1], 1, y_size, fp_src); // U
            fread(pPic_in->planes[2], 1, y_size, fp_src); // V
            break;
        }
        case X265_CSP_I420: {
            fread(pPic_in->planes[0], 1, y_size, fp_src);     // Y
            fread(pPic_in->planes[1], 1, y_size / 4, fp_src); // U
            fread(pPic_in->planes[2], 1, y_size / 4, fp_src); // V
            break;
        }
        default: {
            printf("Colorspace Not Support.\n");
            return -1;
        }
        }

        ret = x265_encoder_encode(pHandle, &pNals, &iNal, pPic_in, NULL); // 不需要重建后的图片可以传递NULL
        printf("Succeed encode %5d frames iNal: %d\n", i, iNal);

        for (j = 0; j < iNal; j++) {
            fwrite(pNals[j].payload, 1, pNals[j].sizeBytes, fp_dst);
        }
    }
    // Flush Decoder
    printf("flush encoder\n");
    while (1) {
        ret = x265_encoder_encode(pHandle, &pNals, &iNal, NULL, NULL);
        if (ret == 0) {
            break;
        }
        printf("Flush 1 frame.\n");

        for (j = 0; j < iNal; j++) {
            fwrite(pNals[j].payload, 1, pNals[j].sizeBytes, fp_dst);
        }
    }

    x265_encoder_close(pHandle);
    x265_picture_free(pPic_in);
    x265_param_free(pParam);
    free(buff);
    fclose(fp_src);
    fclose(fp_dst);

    return 0;
}