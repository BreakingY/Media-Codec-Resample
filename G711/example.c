#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "g711_table.h"

void print_usage(char *program_name)
{
    printf("Usage: %s input_file CONVERSION output_file\n", program_name);
    printf("Supported CONVERSIONs: pcm_alaw, pcm_ulaw, alaw_pcm, ulaw_pcm\n");
}

long get_file_size(FILE *f)
{
    long file_size;

    /* Go to end of file */
    fseek(f, 0L, SEEK_END);

    /* Get the number of bytes */
    file_size = ftell(f);

    /* reset the file position indicator to 
    the beginning of the file */
    fseek(f, 0L, SEEK_SET);	

    return file_size;
}

char * allocate_buffer(long buffer_size)
{
    char *buffer;

    /* grab sufficient memory for the 
    buffer to hold the audio */
    buffer = (char*)calloc(buffer_size, sizeof(char));	
    /* memory error */
    if(buffer == NULL)
    {
        perror("Error while allocating memory for write buffer.\n");
        exit(EXIT_FAILURE);
    }

    return buffer;
}

int main(int argc, char *argv[])
{
    FILE    *fRead, *fWrite;
    char    *bufferRead, *bufferWrite;
    long    bufferReadSize, bufferWriteSize;
    size_t  readed;

    if(argc != 4)
    {
        printf("Incorrect parameter length.\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    /* open an existing file for reading */
    fRead = fopen(argv[1],"rb");

    /* quit if the file does not exist */
    if( fRead == NULL )
    {
       perror("Error while opening read file.\n");
       exit(EXIT_FAILURE);
    }

    /* Get file size */
    bufferReadSize = get_file_size(fRead);

    /* grab sufficient memory for the 
    buffer to hold the audio */
    bufferRead = allocate_buffer(bufferReadSize);

    /* copy all the text into the buffer */
    readed = fread(bufferRead, sizeof(char), bufferReadSize, fRead);
    if (readed != bufferReadSize)
    {
        perror("Incorrect bytes readed\n");
        exit(EXIT_FAILURE);
    }
    fclose(fRead);

    /* Conversions */

    if (strcmp(argv[2], "alaw_pcm") == 0) //alaw to pcm
    {
        alaw_pcm16_tableinit();
        bufferWriteSize = bufferReadSize * 2;
        bufferWrite = allocate_buffer(bufferWriteSize);
        alaw_to_pcm16(bufferReadSize, bufferRead, bufferWrite);
    } 
    else if (strcmp(argv[2], "ulaw_pcm") == 0)//ulaw to pcm
    {
        ulaw_pcm16_tableinit();
        bufferWriteSize = bufferReadSize * 2;
        bufferWrite = allocate_buffer(bufferWriteSize);
        ulaw_to_pcm16(bufferReadSize, bufferRead, bufferWrite);
    }
    else if (strcmp(argv[2], "pcm_alaw") == 0)//pcm to alaw
    {
        pcm16_alaw_tableinit();
        bufferWriteSize = bufferReadSize / 2;
        bufferWrite = allocate_buffer(bufferWriteSize);
        pcm16_to_alaw(bufferReadSize, bufferRead, bufferWrite);
    }
    else if (strcmp(argv[2], "pcm_ulaw") == 0)//pcm to ulaw
    {
        pcm16_ulaw_tableinit();
        bufferWriteSize = bufferReadSize / 2;
        bufferWrite = allocate_buffer(bufferWriteSize);
        pcm16_to_ulaw(bufferReadSize, bufferRead, bufferWrite);
    }
    else
    {
        perror("Incorrect parameter.\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Bytes read: %ld, Buffer Write: %ld\n", bufferReadSize, bufferWriteSize);

    /* open a file for writing */
    fWrite = fopen(argv[3], "wb");

    /* quit if the file can not be opened */
    if( fWrite == NULL )
    {
       perror("Error while opening the write file.\n");
       exit(EXIT_FAILURE);
    }    

    /* copy all the buffer into the file */
    fwrite (bufferWrite , sizeof(char), bufferWriteSize, fWrite);
    fclose (fWrite);

    /* free the memory we used for the buffer */
    free(bufferWrite);
    free(bufferRead);

    return 0;
}