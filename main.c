/*
 * Project Description:
 * Implement image steganography using the LSB (Least Significant Bit)
 * technique in C to hide secret file data inside a BMP image and
 * retrieve the original data through decoding.
 */

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "common.h"

int main(int argc,char *argv[])
{
    
    OperationType result = check_operation_type(argv);
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    if(argc < 2)
    {
        help_stegno();
        return 1;
    }

    if(result == e_encode)
    {
        if( argc < 4 || argc > 5)
        {
            printf("Error: encoding needs <source.bmp> <secret_file> [output.bmp]\n");
            return 1;
        }
        if(read_and_validate_encode_args_e(argv,&encInfo) == e_success)
        {
            if(do_encoding(&encInfo) == e_failure)
            printf("Encoding failed!\n");
        }
        else{
            printf("Failed to Encode\n");
            return 1;
        }

       
    }
    else if(result == e_decode)
    {
         if (argc < 3 || argc > 4)
        {
            printf("Error: decoding needs <encoded.bmp> [output_file]\n");
            return 1;
        }

        if(read_and_validate_encode_args_d(argv,&decInfo) == e_success)
        {
            if(do_decoding(&decInfo) == e_failure)
            printf("Decoding failed!\n");;
        }
        else
        {
            printf("Failed to Decode\n");
            return 1;
        }
        
    }
    else
    {
        return (strcmp(argv[1], "-h") == 0)? 0:1;
    }
    return 0;
}
