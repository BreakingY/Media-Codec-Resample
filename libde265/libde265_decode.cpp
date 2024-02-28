#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <libde265/de265.h>
static inline int startCode3(unsigned char *buf)
{
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return 1;
    else
        return 0;
}

static inline int startCode4(unsigned char *buf)
{
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return 1;
    else
        return 0;
}

static unsigned char *findNextStartCode(unsigned char *buf, int len)
{
    int i;

    if (len < 3)
        return NULL;

    for (i = 0; i < len - 3; ++i) {
        if (startCode3(buf) || startCode4(buf))
            return buf;

        ++buf;
    }

    if (startCode3(buf))
        return buf;

    return NULL;
}

static int getFrameFromH264File(FILE *file, unsigned char *frame, int size)
{
    int rSize, frameSize;
    unsigned char *nextStartCode;

    rSize = fread(frame, 1, size, file);
    if (!startCode3(frame) && !startCode4(frame))
        return -1;

    nextStartCode = findNextStartCode(frame + 3, rSize - 3);
    if (!nextStartCode) {
        // fseek(file, 0, SEEK_SET);
        // frameSize = rSize;
        return -1;
    } else {
        frameSize = (nextStartCode - frame);
        fseek(file, frameSize - rSize, SEEK_CUR);
    }

    return frameSize;
}
int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("./bin input ouput\n");
        return -1;
    }
    FILE *inFile = fopen(argv[1], "rb");
    if (!inFile) {
        printf("failed to open %s\n", argv[1]);
        return -1;
    }
    FILE *outFile = fopen(argv[2], "wb");
    if (!outFile) {
        printf("failed to open %s\n", argv[2]);
        fclose(inFile);
        return -1;
    }
    int len = 4 * 1024 * 1024;
    unsigned char *frame = (unsigned char *)malloc(len);

    de265_error err;
    de265_decoder_context *hdecoder = de265_new_decoder();
    int number_of_threads = 1;
    de265_start_worker_threads(hdecoder, number_of_threads);
    int64_t pts = 0;
    while (true) {
        int frameSize = getFrameFromH264File(inFile, frame, len);
        if (frameSize < 0) {
            printf("file over\n");
            break;
        }
        int type;
        if (startCode3(frame)) {
            type = (frame[3] >> 1) & 0x3f;
        } else if (startCode4(frame)) {
            type = (frame[4] >> 1) & 0x3f;
        } else {
            printf("start code error\n");
        }
        printf("type:%d\n", type);

        err = de265_push_data(hdecoder, frame, frameSize, pts, nullptr);

        if (err != DE265_OK) {
            break;
        }
        // 降低解码时延，指示解码器，放入的是一帧完整的图像，解码立即输出
        de265_push_end_of_NAL(hdecoder);
        de265_push_end_of_frame(hdecoder);

        // decode
        while (true) {
            /*
            - decoding successful   -> err  = DE265_OK, more=true
            - decoding stalled      -> err != DE265_OK, more=true
            - decoding finished     -> err  = DE265_OK, more=false
            - unresolvable error    -> err != DE265_OK, more=false
            */
            int more = true;
            err = de265_decode(hdecoder, &more);
            if (err != DE265_OK && more == false) {
                break;
            }
            const struct de265_image *img = de265_get_next_picture(hdecoder);
            if (img == nullptr) {
                break;
            }
            enum de265_chroma format = de265_get_chroma_format(img);
            if (format == de265_chroma_420) {
                printf("de265_chroma_420\n");
            }
            de265_PTS pts_img = de265_get_image_PTS(img);
            std::cout << "pts_img:" << pts_img << "pts:" << pts << std::endl;
            for (int i = 0; i < 3; ++i) {
                int stride;
                const uint8_t *data = de265_get_image_plane(img, i, &stride);
                int width = de265_get_image_width(img, i);
                int height = de265_get_image_height(img, i);
                printf("i:%d stride:%d\n", i, stride);
                for (int y = 0; y < height; y++) {
                    fwrite(reinterpret_cast<const char *>(data) + y * stride, 1, width, outFile);
                    fflush(outFile);
                }
            }
        }
        pts++;
    }

    err = de265_flush_data(hdecoder);
    // decode
    while (true) {
        /*
        - decoding successful->err = DE265_OK, more = true
        - decoding stalled->err != DE265_OK, more = true
        - decoding finished->err = DE265_OK, more = false
        - unresolvable error->err != DE265_OK, more = false
        */
        int more = true;
        err = de265_decode(hdecoder, &more);
        if (err != DE265_OK && more == false) {
            break;
        }

        const struct de265_image *img = de265_get_next_picture(hdecoder);
        if (img == nullptr) {
            break;
        }

        for (int i = 0; i < 3; ++i) {
            int stride;
            const uint8_t *data = de265_get_image_plane(img, i, &stride);
            int width = de265_get_image_width(img, i);
            int height = de265_get_image_height(img, i);
            for (int y = 0; y < height; y++) {
                fwrite(reinterpret_cast<const char *>(data) + y * stride, 1, width, outFile);
                fflush(outFile);
            }
        }
    }

    err = de265_free_decoder(hdecoder);

    fclose(outFile);
    fclose(inFile);
    free(frame);
    return 0;
}