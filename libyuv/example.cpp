#include "libyuv.h"
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("only support yuv420p test\n./bin input width height\n");
        return 1;
    }
    FILE *fp_src = fopen(argv[1], "rb");
    FILE *fp_dst = fopen("RGB.yuv", "wb");
    if (fp_src == NULL || fp_dst == NULL) {
        printf("Error open files.\n");
        return -1;
    }
    int picWidth = atoi(argv[2]);
    int picHeight = atoi(argv[3]);
    int frame_size = picWidth * picHeight * 3 / 2;
    unsigned char *data = new unsigned char[frame_size];
    unsigned char *dst_rgb = new unsigned char[picWidth * picHeight * 3];
    int dst_stride_rgb = picWidth * 3;
    int num = 60;
    FILE *fp_cmp = fopen("cmp_yuv420p.yuv", "wb");
    unsigned char *data_cmp = new unsigned char[frame_size];

    // scale
    int scale_width = 1280;
    int scale_height = 720;
    unsigned char *data_scale = new unsigned char[scale_width * scale_height * 3 / 2];
    unsigned char *data_rotate = new unsigned char[scale_width * scale_height * 3 / 2];
    unsigned char *data_mirror = new unsigned char[scale_width * scale_height * 3 / 2];
    FILE *fp_scale = fopen("scale_yuv420p_1280_720.yuv", "wb");
    while (num-- >= 0) {
        if (fread(data, 1, frame_size, fp_src) <= 0) {
            std::cout << "file over" << std::endl;
            break;
        }
        // convert
        const uint8_t *src_y = data;
        int src_stride_y = picWidth;
        const uint8_t *src_u = data + picWidth * picHeight;
        int src_stride_u = picWidth / 2;
        const uint8_t *src_v = data + picWidth * picHeight + (picWidth * picHeight) / 4;
        int src_stride_v = picWidth / 2;
        // yuvplayer播放RGB时候是按照BGR播放的，这是yuvplayer或者windows显示的问题，转换没有问题(回转cmp.yuv中的yuv420p播放色彩是正常的)
        libyuv::I420ToRGB24(src_y, src_stride_y, src_u, src_stride_u, src_v, src_stride_v,
                            dst_rgb, dst_stride_rgb, picWidth, picHeight);
        fwrite(dst_rgb, 1, picWidth * picHeight * 3, fp_dst);

        libyuv::RGB24ToI420(dst_rgb, dst_stride_rgb,
                            data_cmp, src_stride_y,
                            data_cmp + picWidth * picHeight, src_stride_u,
                            data_cmp + picWidth * picHeight + (picWidth * picHeight) / 4, src_stride_v,
                            picWidth, picHeight);
        fwrite(data_cmp, 1, picWidth * picHeight * 3 / 2, fp_cmp);

        // scale
        libyuv::I420Scale(data_cmp, src_stride_y,
                          data_cmp + picWidth * picHeight, src_stride_u,
                          data_cmp + picWidth * picHeight + (picWidth * picHeight) / 4, src_stride_v,
                          picWidth, picHeight,
                          data_scale, scale_width,
                          data_scale + scale_width * scale_height, scale_width / 2,
                          data_scale + scale_width * scale_height + (scale_width * scale_height) / 4, scale_width / 2,
                          scale_width, scale_height,
                          libyuv::kFilterBilinear);
        // rotate
        // 要根据旋转之后的布局调整data_rotate的宽高，例如kRotate90 就是把宽变成高，把高变成宽，kRotate180相当于上下颠倒，宽高不变
        libyuv::I420Rotate(data_scale, scale_width,
                           data_scale + scale_width * scale_height, scale_width / 2,
                           data_scale + scale_width * scale_height + (scale_width * scale_height) / 4, scale_width / 2,
                           data_rotate, scale_width,
                           data_rotate + scale_width * scale_height, scale_width / 2,
                           data_rotate + scale_width * scale_height + (scale_width * scale_height) / 4, scale_width / 2,
                           scale_width, scale_height,
                           libyuv::kRotate0);
        // 绘制矩形框
        libyuv::I420Rect(data_rotate, scale_width,
                         data_rotate + scale_width * scale_height, scale_width / 2,
                         data_rotate + scale_width * scale_height + (scale_width * scale_height) / 4, scale_width / 2,
                         20, 20,
                         100, 100,
                         255, 255, 255);
        // 镜像
        libyuv::I420Mirror(data_rotate, scale_width,
                           data_rotate + scale_width * scale_height, scale_width / 2,
                           data_rotate + scale_width * scale_height + (scale_width * scale_height) / 4, scale_width / 2,
                           data_mirror, scale_width,
                           data_mirror + scale_width * scale_height, scale_width / 2,
                           data_mirror + scale_width * scale_height + (scale_width * scale_height) / 4, scale_width / 2,
                           scale_width, scale_height);

        fwrite(data_mirror, 1, scale_width * scale_height * 3 / 2, fp_scale);
    }
    delete[] data;
    delete[] dst_rgb;
    delete[] data_cmp;
    delete[] data_scale;
    delete[] data_rotate;
    delete[] data_mirror;
    fclose(fp_src);
    fclose(fp_dst);
    fclose(fp_cmp);
    fclose(fp_scale);
    return 0;
}