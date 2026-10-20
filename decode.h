#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

typedef struct{
    /*stego image*/
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /*Output file*/
    char output_fname[256];
    FILE *fptr_output;

    char extn_secret_file[10];
    long size_secret_file;

}DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_encode_args_d(char *argv[], DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_files1(char *filename, FILE **fptr ,char *mode);

/* Decode function, which does the real decoding for char data */
char decode_lsb_to_byte(unsigned char data[]);

/* Decode function, which does the real decoding for integer*/
unsigned int decode_lsb_to_int(unsigned char data[]);

/* Decode character data from image */
Status decode_data_from_image(char *data,size_t size,DecodeInfo *decInfo);

/* Decode magic string from image */
Status decode_magic_string(char *magic_string,DecodeInfo *decInfo);

/* Decode secret data from image */
Status decode_secret_data_from_image(unsigned int len,DecodeInfo *decInfo); 

/* Decode integer from image */
Status decode_int_from_image(unsigned int *lsb, DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_file_extn(unsigned int extn_len, DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

#endif