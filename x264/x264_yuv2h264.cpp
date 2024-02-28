#include <stdio.h>
#include <stdlib.h>

#include "stdint.h"

#if defined(__cplusplus)
extern "C" {
#include "x264.h"
};
#else
#include "x264.h"
#endif
int main(int argc, char **argv)
{

    int ret;
    int y_size;
    int i, j;
    if (argc < 6) {
        printf("./bin input ouput width height fps\n");
        return 1;
    }
    FILE *fp_src = fopen(argv[1], "rb");
    FILE *fp_dst = fopen(argv[2], "wb");
    int csp = X264_CSP_I420;

    // Encode frame number
    int frame_num = 0;

    int iNal = 0;
    x264_nal_t *pNals = NULL;
    x264_t *pHandle = NULL;
    x264_picture_t *pPic_in = (x264_picture_t *)malloc(sizeof(x264_picture_t));
    x264_picture_t *pPic_out = (x264_picture_t *)malloc(sizeof(x264_picture_t));
    x264_param_t *pParam = (x264_param_t *)malloc(sizeof(x264_param_t));

    // Check
    if (fp_src == NULL || fp_dst == NULL) {
        printf("Error open files.\n");
        return -1;
    }

    x264_param_default(pParam);                               // 给参数结构体赋默认值
    x264_param_default_preset(pParam, "fast", "zerolatency"); // 设置preset和tune

    // 修改部分参数
    pParam->i_csp = csp;
    pParam->i_width = atoi(argv[3]);   // 宽度
    pParam->i_height = atoi(argv[4]);  // 高度
    pParam->i_fps_num = atoi(argv[5]); // 设置帧率（分子）
    pParam->i_fps_den = 1;             // 设置帧率时间1s（分母）

    pParam->i_threads = X264_SYNC_LOOKAHEAD_AUTO;
    pParam->i_keyint_max = atoi(argv[5]); // 在此间隔设置IDR关键帧

    pParam->rc.i_rc_method = X264_RC_ABR; // 码率控制方法，CQP(恒定质量)，CRF(恒定码率,缺省值23)，ABR(平均码率)
    pParam->rc.i_bitrate = 2048;          // 设置码率,在ABR(平均码率)模式下才生效，且必须在设置ABR前先设置bitrate
    // x264码流控制需对i_vbv_max_bitrate i_vbv_buffer_size赋值
    pParam->rc.i_vbv_max_bitrate = 2048; // 平均码率模式下的，最大瞬时码率
    pParam->rc.i_vbv_buffer_size = 2048; // 调整的时候可以没有这个参数，初始化的时候必须有，否则无法起到调整码率的作用！！
    pParam->rc.f_rate_tolerance = 1;     // 平均码率的模式下，瞬时码率可以偏离的倍数，范围是0.1 ~100,默认是1.0

    // ABR模式下调整i_bitrate，
    // CQP下调整i_qp_constant调整QP值，范围0~51，值越大图像越模糊，默认23
    // CRF下调整f_rf_constant和f_rf_constant_max影响编码速度和图像质量（数据量）；

    pParam->b_repeat_headers = 1; // put SPS/PPS before each keyframe
    pParam->b_annexb = 1;         // 决定NALU前４个字节，是用start codes(1)，还是size nalu(0)

#if 0
    //完全重构，并写道文件中，yuv格式的视频
    pParam->b_full_recon=1;
    pParam->psz_dump_yuv="psz_dump_yuv.yuv";
#endif

    // set profile
    x264_param_apply_profile(pParam, "baseline"); // 这里是main profile分支的Constrained Baseline而不是extend分支的baseline Constrained Baseline没有B帧

    // open encoder
    pHandle = x264_encoder_open(pParam);

    x264_picture_init(pPic_out);
    x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);

    // ret = x264_encoder_headers(pHandle, &pNals, &iNal);//可以获取编码参数 sps pps

    y_size = pParam->i_width * pParam->i_height;
    // detect frame number

    fseek(fp_src, 0, SEEK_END);
    switch (csp) {
    case X264_CSP_I444:
        frame_num = ftell(fp_src) / (y_size * 3);
        break;
    case X264_CSP_I422:
        frame_num = ftell(fp_src) / (y_size * 2);
        break;
    case X264_CSP_I420:
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
        case X264_CSP_I444: {
            fread(pPic_in->img.plane[0], y_size, 1, fp_src); // Y
            fread(pPic_in->img.plane[1], y_size, 1, fp_src); // U
            fread(pPic_in->img.plane[2], y_size, 1, fp_src); // V
            break;
        }
        case X264_CSP_I422: {
            int index = 0;
            int y_i = 0, u_i = 0, v_i = 0;
            for (index = 0; index < y_size * 2;) {
                fread(&pPic_in->img.plane[0][y_i++], 1, 1, fp_src); // Y
                index++;
                fread(&pPic_in->img.plane[1][u_i++], 1, 1, fp_src); // U
                index++;
                fread(&pPic_in->img.plane[0][y_i++], 1, 1, fp_src); // Y
                index++;
                fread(&pPic_in->img.plane[2][v_i++], 1, 1, fp_src); // V
                index++;
            }
            break;
        }

        case X264_CSP_I420: {
            fread(pPic_in->img.plane[0], y_size, 1, fp_src);     // Y
            fread(pPic_in->img.plane[1], y_size / 4, 1, fp_src); // U
            fread(pPic_in->img.plane[2], y_size / 4, 1, fp_src); // V
            break;
        }
        default: {
            printf("Colorspace Not Support.\n");
            return -1;
        }
        }
        pPic_in->i_pts = i;

        ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out); // pPic_out ：通常是编码过程中该帧编码后重建的yuv。
        if (ret < 0) {
            printf("Error.\n");
            return -1;
        }

        printf("Succeed encode frame: %5d iNal: %d\n", i, iNal);

        for (j = 0; j < iNal; ++j) {
            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
        }
    }
    i = 0;
    // flush encoder
    printf("flush encoder\n");
    while (1) {
        ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);
        if (ret == 0) {
            break;
        }
        printf("Flush 1 frame.\n");
        for (j = 0; j < iNal; ++j) {
            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);
        }
        i++;
    }
    x264_picture_clean(pPic_in);
    x264_encoder_close(pHandle);
    pHandle = NULL;

    free(pPic_in);
    free(pPic_out);
    free(pParam);

    fclose(fp_src);
    fclose(fp_dst);

    return 0;
}