#include <stdio.h>
#include <unistd.h>
#include "types.h"
#include "common.h"
#include <string.h>

/*
 * Check operation type
 * Input: Command-line arguments (argv[])
 * Output: e_encode for encoding, e_decode for decoding,
 *         or e_unsupported for an invalid operation
 * Description: Check the operation specified by the user.
 *              Return e_encode if the argument is "-e",
 *              e_decode if the argument is "-d", and
 *              e_unsupported for any other argument.
 *              Display the usage instructions for invalid operations.
 */
OperationType check_operation_type(char *argv[])
{
    if (argv[1] == NULL)
    {
        return e_unsupported;
    }
    if(strcmp(argv[1],"-e")==0){
        return e_encode;
    }
    else if(strcmp(argv[1],"-d")==0){
        return e_decode;
    }
    else{
        help_stegno();
        return e_unsupported;
    }


}

/*
 * Display usage instructions
 * Input: None
 * Output: None
 * Description: Display the usage instructions for the
 *              Steganography application, including the
 *              commands and arguments required for encoding,
 *              decoding, and displaying help.
 */

void help_stegno()
{
    printf("\n");
    printf("============================================================\n");
    printf("%40s\n", "LSB IMAGE STEGANOGRAPHY");
    printf("============================================================\n");

    printf("\nDESCRIPTION:\n");
    printf("  This project hides secret data inside a BMP image using\n");
    printf("  the Least Significant Bit (LSB) steganography technique.\n");
    printf("  It also extracts the hidden data from an encoded image.\n");

    printf("\nUSAGE:\n");
    printf("  ./a.out <option> [arguments]\n");

    printf("\nOPTIONS:\n");
    printf("  -e    Encode a secret file into a BMP image.\n");
    printf("  -d    Decode hidden data from a stego image.\n");
    printf("  -h    Display this help menu.\n");

    printf("\nENCODING:\n");
    printf("  Syntax:\n");
    printf("    ./a.out -e <source_image.bmp> <secret_file> [output_image]\n");
    printf("\n");
    printf("  Arguments:\n");
    printf("    source_image.bmp  Original BMP image.\n");
    printf("    secret_file       File to hide inside the image.\n");
    printf("    output_image      Optional output BMP filename.\n");
    printf("                      Default: stego.bmp\n");
    printf("                      If extension is omitted, .bmp is added.\n");

    printf("\n  Example:\n");
    printf("    ./a.out -e input.bmp secret.txt output.bmp\n");
    printf("    ./a.out -e input.bmp secret.txt output\n");

    printf("\nDECODING:\n");
    printf("  Syntax:\n");
    printf("    ./a.out -d <encoded_image.bmp> [output_file]\n");
    printf("\n");
    printf("  Arguments:\n");
    printf("    encoded_image.bmp  Image containing hidden data.\n");
    printf("    output_file        Optional name for the extracted file.\n");

    printf("\n  Example:\n");
    printf("    ./a.out -d stego.bmp\n");
    printf("    ./a.out -d stego.bmp recovered.txt\n");

    printf("\nSUPPORTED FILES:\n");
    printf("  Source image : .bmp\n");
    printf("  Secret file  : Any file type supported by the program.\n");
    printf("  Output image : .bmp\n");

    printf("\nNOTES:\n");
    printf("  * Ensure the source image has sufficient capacity.\n");
    printf("  * The output image must have a .bmp extension.\n");
    printf("  * Use -h to display this help menu at any time.\n");

    printf("\n============================================================\n\n");
   
}





void print_message(const char *message)
{
    printf("%s\n", message);
    fflush(stdout);
    sleep(1);
}
