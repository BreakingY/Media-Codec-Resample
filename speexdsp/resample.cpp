#include <cmath>
#include <iostream>
#include <memory>
#include <speex/speex_resampler.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// 16bits 24bits 32bitsPCM都是有符号的，只有8bits没有负数，全是正数
void convert16to8(const short *input, unsigned char *output, int numSamples)
{
    int audioSum = 0;
    for (int i = 0; i < numSamples; i++) {
#if 1
        int value = (input[i] + 32768) / 256; // Convert 16-bit range to 8-bit range
#else
        // 这种方法有杂音
        int value = input[i] >> 8;
        value += 128;
#endif
        output[i] = (unsigned char)value;
        audioSum += abs(input[i]);
    }
    if (audioSum < numSamples * 10) {      // 声音太小或者是静音,则清空output，否则有杂音
        memset(output, 0, numSamples * 1); // 8bits=1byte
    }
}
// 转到8bits才需要判断声音太小或者静音，其他的不用
void convert16to32(const short *input, int *output, int numSamples)
{
    for (int i = 0; i < numSamples; i++) {
        output[i] = (int)input[i] << 16; // Convert 16-bit range to 32-bit range
    }
}
void convert32to16(const int *input, short *output, int numSamples)
{
    for (int i = 0; i < numSamples; i++) {
        output[i] = input[i] >> 16; // Convert 32-bit range to 16-bit range
    }
}

int main(int argc, char **argv)
{
    if (argc < 9) {
        printf("speexdsp only support rate change\n./bin input output in_channel in_rate in_bits(must be 16bits) out_channel out_rate out_bits(must be 16bits)\n");
        return -1;
    }
    char *input = argv[1];
    char *output = argv[2];
    int in_channel = atoi(argv[3]);
    int in_rate = atoi(argv[4]);
    int in_bits = atoi(argv[5]);
    int out_channel = atoi(argv[6]);
    int out_rate = atoi(argv[7]);
    int out_bits = atoi(argv[8]);
    printf("in_channel:%d in_rate:%d in_bits:%d\n", in_channel, in_rate, in_bits);
    printf("out_channel:%d out_rate:%d out_bits:%d\n", out_channel, out_rate, out_bits);

    SpeexResamplerState *resampler = NULL;
    int err = 0;

    resampler = speex_resampler_init(in_channel, in_rate, out_rate, 10, &err);
    speex_resampler_set_rate(resampler, in_rate, out_rate);
    // speex_resampler_skip_zeros(resampler);//优化用的，但是会引入时延

    FILE *pcmfd = fopen(input, "r");
    if (!pcmfd) {
        // cout << "open test.cpm faile" << endl;
        printf("openinput failed\n");
        return -1;
    }
    FILE *outfd = fopen(output, "w");
    if (!outfd) {
        fclose(pcmfd);
        printf("open output failed\n");
        return -1;
    }
    int in_samples = 1024;
    // ffmpeg->dst_nb_samples = av_rescale_rnd(src_nb_samples, dst_ratio, src_ratio, AV_ROUND_UP);
    double resampling_ratio = (double)out_rate / in_rate;
    int out_samples = (int)ceil(in_samples * resampling_ratio); // 乘法的时候in_samples会自动转换成double，ceil向上取整
    printf("in_samples:%d out_samples:%d resampling_ratio:%lf\n", in_samples, out_samples, resampling_ratio);

    int buffer_size = in_samples * in_channel * (in_bits / 8);
    int out_buffer_size = out_samples * out_channel * (out_bits / 8);
    unsigned char *pcmbuffer = (unsigned char *)malloc(buffer_size);
    unsigned char *out_buffer = (unsigned char *)malloc(out_buffer_size);

    // change bits
    int out_buffer_size_8bits = out_buffer_size / 2;
    unsigned char *out_buffer_8bits = (unsigned char *)malloc(out_buffer_size_8bits);
    int out_buffer_size_16bits = out_buffer_size * 2;
    unsigned char *out_buffer_32bits = (unsigned char *)malloc(out_buffer_size_16bits);
    int ret = 0;
    while (true) {
        ret = fread(pcmbuffer, 1, buffer_size, pcmfd);
        if (ret <= 0) {
            printf("file over\n");
            break;
        }
        memset(out_buffer, 0, out_buffer_size);
        unsigned int inlen = ret / in_channel / (in_bits / 8); // in_samples;
        unsigned int outlen = out_buffer_size / sizeof(short);
        printf("before ret:%d inlen:%d outlen:%d\n", ret, inlen, outlen);

        ret = speex_resampler_get_input_latency(resampler);
        printf("input latenc:%d\n", ret);
        ret = speex_resampler_get_output_latency(resampler);
        printf("output latenc:%d\n", ret);
// speex_resampler_get_input_latency speex_resampler_get_output_latency,只是用来表示时延，与ffmpeg中的采样不一样，ffmpeg中期望输出采样点是out_nb_samples，实际返回是rr rr可能不等于out_nb_samples
// speexdsp中outlen是等于out_samples的，不用想ffmpeg那样需要根据swr_get_delay重新计算out_sample_fmt
#if 0
        /*FFmpeg真正进行重采样的函数是swr_convert。它的返回值就是重采样输出的点数。
        使用FFmpeg进行重采样时内部是有缓存的，而内部缓存了多少个采样点，可以用函数swr_get_delay获取。
        也就是说调用函数swr_convert时你传递进去的第三个参数表示你希望输出的采样点数，
        但是函数swr_convert的返回值才是真正输出的采样点数，这个返回值一定是小于或等于你希望输出的采样点数。
        */
        int64_t delay = swr_get_delay(self->au_convert_ctx, self->audioFrame->sample_rate);
        int64_t real_dst_nb_samples = av_rescale_rnd(delay + self->audioFrame->nb_samples, self->out_sample_rate, 
                                        self->audioFrame->sample_rate,AV_ROUND_UP);
        if (real_dst_nb_samples>self->out_nb_samples) {
            printf("%s:%d 重新分配buffer",__FILE__, __LINE__);
            self->out_nb_samples = real_dst_nb_samples;
            av_free(self->out_buffer_aud);
            self->out_buffer_aud_size = av_samples_get_buffer_size(nullptr, self->out_channels, self->out_nb_samples, self->out_sample_fmt, 1);
            self->out_buffer_aud=(uint8_t *)av_malloc(self->out_buffer_aud_size);
            
        }
        int rr = swr_convert(self->au_convert_ctx, &self->out_buffer_aud, self->out_nb_samples,
                                (const uint8_t **)self->audioFrame->data, self->audioFrame->nb_samples);
#endif
        // inlen是输入的单个通道的样本个数，输出是已经重采样样本的个数 outlen输入的是out_buffer的大小,注意是转换成short之后的大小，输出outlen也是单个通道的样本个数
        // 带有interleaved表示多通道交错排列，交错模式 不带interleaved非交错排列
        ret = speex_resampler_process_interleaved_int(resampler, (short *)pcmbuffer, &inlen, (short *)out_buffer, &outlen); // 只能进行16bit的音频重采样，64位深使用speex_resampler_process_interleaved_float
        if (ret == RESAMPLER_ERR_SUCCESS) {
#if 1
            // 16bites to 8bits
            /*
            memset(out_buffer_8bits,0,out_buffer_size_8bits);
            convert16to8((const short *)out_buffer,out_buffer_8bits , outlen*out_channel);
            fwrite(out_buffer_8bits, 1, out_buffer_size_8bits, outfd);
            */
            // 16bits to 32bits
            /*
            memset(out_buffer_32bits,0,out_buffer_size_16bits);
            convert16to32((const short *)out_buffer,(int *)out_buffer_32bits, outlen*out_channel);
            fwrite(out_buffer_32bits, 1, out_buffer_size_16bits, outfd);
            */
            // 16bits to 32bits to 16bits
            memset(out_buffer_32bits, 0, out_buffer_size_16bits);
            convert16to32((const short *)out_buffer, (int *)out_buffer_32bits, outlen * out_channel);
            convert32to16((const int *)out_buffer_32bits, (short *)out_buffer, outlen * out_channel);
            fwrite(out_buffer, 1, outlen * (out_bits / 8) * out_channel, outfd);
#else
            fwrite(out_buffer, 1, outlen * (out_bits / 8) * out_channel, outfd);
#endif
            printf("after inlen:%d outlen:%d\n", inlen, outlen);
        } else {
            printf("error: %d\n", ret);
        }
    }
    speex_resampler_destroy(resampler);
    free(pcmbuffer);
    free(out_buffer);
    free(out_buffer_8bits);
    free(out_buffer_32bits);
    fclose(outfd);
    fclose(pcmfd);
    return 0;
}
