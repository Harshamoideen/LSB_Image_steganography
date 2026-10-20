#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/*
 * Read and validate encoding arguments
 * Input: Command-line arguments (argv[]) and EncodeInfo structure pointer
 * Output: Status (success or failure)
 * Description: Validate the command-line arguments required for encoding.
 *              Check whether the source image has a .bmp extension,
 *              the secret file has a valid extension, and the output
 *              file name is valid. Store the validated file names
 *              in the EncodeInfo structure.
 */

Status read_and_validate_encode_args_e(char *argv[], EncodeInfo *encInfo)
{
	/*source imgae must be a .bmp*/
	char *dot=strrchr(argv[2],'.');
	if(dot == NULL || strcmp(dot,".bmp") !=0)
	{
		printf("Error: source image must be a .bmp file\n");
		return e_failure;

	}
	encInfo->src_image_fname = argv[2];

	char *dot1=strrchr(argv[3],'.');
	if(dot1 == NULL)
	{
		printf("Error: secrete filr has no extension\n");
		return e_failure;
	}
	if(strlen(dot1+1) > sizeof(encInfo->extn_secret_file))
	{
		printf("Error: secrete file extension is too long\n");
		return e_failure;
	}

	strcpy(encInfo->extn_secret_file,dot1+1);
	encInfo->secret_fname = argv[3];

	/* optional output image name, default "stego.bmp" */
	if(argv[4]!=NULL)
	{
		
		if(argv[4] == encInfo->src_image_fname){
			printf("output file and source image file can't be same.\n");
			return e_failure;
		}
		if(strlen(argv[4]) <=4 && strchr(argv[4],'.') != NULL)
		{
			printf("Error: output filename must contain a name before the extension\n");
			return e_failure;
		}
		char *dot2=strrchr(argv[4],'.');
		if(dot2 == NULL || strcmp(dot2, ".bmp") != 0)
		{
			printf("Error: output image must have a .bmp extension\n");
			return e_failure;

		} 
		encInfo->stego_image_fname = argv[4];
	}
	else
	{
		encInfo->stego_image_fname = "stego.bmp";
	}
	return e_success;
}

/*
 * Get image capacity
 * Input : Image file ptr
 * Output: width * height * 3 (every colour byte can hide 1 bit), or 0 if the
 *         file is not a plain 24-bit uncompressed BMP with a 54 byte header
 * Description: In a BMP, width is stored at offset 18 and height after it,
 *              each 4 bytes (little endian).
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
	uint width, height;
	// Seek to 18th byte
	fseek(fptr_image, 18, SEEK_SET);

	// Read the width (an int)
	fread(&width, sizeof(int), 1, fptr_image);
	printf("width = %u\n", width);

	// Read the height (an int)
	fread(&height, sizeof(int), 1, fptr_image);
	printf("height = %u\n", height);

	//rewind
	rewind(fptr_image);

	// Return image capacity
	return width * height * 3;
}

/*
 * Get secret file size
 * Input: EncodeInfo structure pointer
 * Output: Size of the secret file in bytes
 * Description: Move the secret file pointer to the end of the file,
 *              determine its size using ftell(), and store the size
 *              in the EncodeInfo structure.
 */
uint get_file_size(EncodeInfo *encInfo)
{
	fseek(encInfo->fptr_secret,0,SEEK_END);
	size_t file_size=ftell(encInfo->fptr_secret);
	encInfo->size_secret_file = file_size;
	return file_size;
}
/*
 * Open files
 * Input: EncodeInfo structure pointer
 * Output: Status (success or failure)
 * Description: Open the source BMP image and secret file in read mode.
 *              Open the output stego image in write mode.
 *              Return failure if any required file cannot be opened.
 */

Status open_files(EncodeInfo *encInfo)
{
	// Src Image file
	encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
	// Do Error handling
	if (encInfo->fptr_src_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

		return e_failure;
	}

	// Secret file
	encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
	// Do Error handling
	if (encInfo->fptr_secret == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

		return e_failure;
	}

	// Stego Image file
	encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
	// Do Error handling
	if (encInfo->fptr_stego_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

		return e_failure;
	}

	// No failure return e_success
	return e_success;
}

/*
 * Check encoding capacity
 * Input: EncodeInfo structure pointer
 * Output: Status (success or failure)
 * Description: Calculate the image capacity and the storage required
 *              for the secret file. Verify whether the source image
 *              has enough capacity to store the secret data.
 */

Status check_capacity(EncodeInfo *encInfo)
{
	fseek(encInfo->fptr_secret,0,SEEK_END);
	encInfo->size_secret_file = ftell(encInfo->fptr_secret);
	rewind(encInfo->fptr_secret);
	size_t size_of_info = strlen(MAGIC_STRING) + sizeof(int) + strlen(encInfo->extn_secret_file) + sizeof(int) + encInfo->size_secret_file;
	size_t image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

	if(image_capacity == 0)
	{
		printf("Error: Invalid BMP image\n");
		return e_failure;
	}

	if(image_capacity >= (size_of_info * 8)){
		return e_success;
	}
	printf("Error: .bmp has no capacity for this data\n");
	return e_failure;
}

/*
 * Copy BMP header
 * Input: Source image file pointer and destination image file pointer
 * Output: e_success if the header is copied successfully,
 *         otherwise e_failure
 * Description: Copy the BMP header from the source image to the
 *              destination image without modifying its contents.
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
	unsigned char arr[BMP_HEADER_SIZE];

	rewind(fptr_src_image);

	if (fread(arr, BMP_HEADER_SIZE, 1, fptr_src_image) != 1 ||
			fwrite(arr, BMP_HEADER_SIZE, 1, fptr_dest_image) != 1)
	{
		printf("Error: could not copy BMP header\n");
		return e_failure;
	}

	return e_success;
}

/* Hide the magic string ("#*") so the decoder can recognise our images */
Status encode_magic_string( char *magic_string, EncodeInfo *encInfo)
{
	size_t size = strlen(MAGIC_STRING);
	encode_data_to_image(magic_string,size,encInfo->fptr_src_image,encInfo->fptr_stego_image);

	return e_success;
}

/* Hide the secret file extension: first its length (32 bits) then the characters */
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
	int length = strlen(file_extn);
	if(length > MAX_FILE_SUFFIX)
	{
      printf("Error: exceeded the maximum file extension length.\n");
	  return e_failure;
	}
	encode_data_int_to_image(length,encInfo->fptr_src_image,encInfo->fptr_stego_image);
	encode_data_to_image(file_extn,length,encInfo->fptr_src_image,encInfo->fptr_stego_image);

	return e_success;

}

/* Hide the size of the secret file (32 bits) */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{  
	encode_data_int_to_image(encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
	return e_success;
}

/* Read the secret file into memory and hide every byte of it */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
	encInfo->secret_data = malloc(encInfo->size_secret_file);

	if (encInfo->secret_data == NULL)
	{
		printf("Memory allocation failed\n");
		return e_failure;
	}
	fseek(encInfo->fptr_secret,0,SEEK_SET);
	fread(encInfo->secret_data,encInfo->size_secret_file,1,encInfo->fptr_secret);
	encode_data_to_image(encInfo->secret_data,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
	fwrite(encInfo->secret_data,encInfo->size_secret_file,1,encInfo->fptr_secret);

	free(encInfo->secret_data);
	encInfo->secret_data = NULL;
	return e_success;
}

/*
 * Hide one byte in the LSBs of 8 image bytes (bit 0 of data -> image_buffer[0], ...)
 */
Status encode_byte_to_lsb(char data, unsigned char *image_buffer)
{
	for(int i = 0; i < 8; i++)
	{
		char get = (data & (1<<i)) >> i;
		image_buffer[i] = image_buffer[i] & ~1;
		image_buffer[i]=image_buffer[i]| get;
	}
	return e_success;
}

/*
 * Hide a 32 bit value in the LSBs of 32 image bytes.
 */
Status encode_byte_int_to_lsb(unsigned int data, unsigned char *image_buffer)
{
	for(int i = 0; i < 32; i++)
	{
		unsigned int get = (data >> i) & 1U;
		image_buffer[i] = image_buffer[i] & 0xFE;
		image_buffer[i] = image_buffer[i]| get;
	}
	return e_success;
}
/* Hide 'size' bytes of 'data': read 8 image bytes per data byte, change LSBs, write them */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
	for(int i=0 ; i < size ;i++)
	{
		unsigned char arr[8];
		fread(arr,8,1,fptr_src_image);
		encode_byte_to_lsb(data[i],arr);
		fwrite(arr,8,1,fptr_stego_image);
	}
	return e_success;
}
/* Hide a 32 bit integer: read 32 image bytes, change LSBs, write them */
Status encode_data_int_to_image(int data, FILE *fptr_src_image, FILE *fptr_stego_image)
{

	unsigned char arr[32];
	fread(arr,32,1,fptr_src_image);
	encode_byte_int_to_lsb(data,arr);
	fwrite(arr,32,1,fptr_stego_image);

	return e_success;
}

/* Copy everything that is left of the source image (untouched pixels) */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
	char buf[4096];
	size_t n;
	while((n = fread(buf,1,sizeof(buf),fptr_src)) > 0)
	{
		if(fwrite(buf,1,n,fptr_dest) != n)
			return e_failure;

	}
	return e_success;
}

/*
 * Perform the complete encoding process
 * Input: EncodeInfo structure pointer
 * Output: e_success if encoding completes successfully,
 *         otherwise e_failure
 * Description: Open the source image, secret file, and destination stego image. 
 *              Check whether the source image has enough capacity to store the 
 *              secret data. Copy the BMP header, encode the magic string, secret 
 *              file extension, secret file size, and secret file data into the 
 *              image. Finally, copy the remaining image data to the destination
 *              image and report the encoding status.
 */

Status do_encoding(EncodeInfo *encInfo)
{
	print_message("INFO: Opening all files in required mode..");
	if( open_files(encInfo) == e_failure)
	{
		printf("Error: Failed to open files\n");
		return e_failure;
	}
	else
	{
		print_message("INFO: Sucessfully opened  all files.\n");
	}
	print_message("INFO: Checking image capacity...");
	if(check_capacity(encInfo) == e_failure)
	{
		printf("Abort due to insufficient capacity\n");
		return e_failure;
	}
	print_message("INFO: Image capacity verified.\n");

	print_message("INFO: Copying Image header...");
	if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure)
	{
		printf("Error: Unable to copy bmp header.\n");
		return e_failure;
	}
	print_message("INFO: Image header copied.\n");
	get_file_size(encInfo);
	print_message("INFO: Encoding Magic string... ");
	if(encode_magic_string(MAGIC_STRING,encInfo) == e_failure)
	{
		printf("Error: Unable to encode magic string.\n");
		return e_failure;
	}
	print_message("INFO: Encoding secret file extension...");

	if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_failure)
	{
		printf("Error: Unable to encode the secret file extension.\n");
		return e_failure;
	}

	print_message("INFO: Encoding secret file size...");

	if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_failure)
	{
		printf("Error: Unable to encode the secret file size.\n");
		return e_failure;
	}
	print_message("INFO: Encoding secret file data...");

	if(encode_secret_file_data(encInfo) == e_failure)
	{
		printf("Error: Unable to encode the secret file data.\n");
		return e_failure;
	}
	print_message("INFO: Copying remaining image data...");

	if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
	{
		printf("Error: Unable to copy remaining image.\n");
		return e_failure;
	}
	printf("\n===============================================\n");
	printf("        Encoding completed successfully!\n");
	printf("===============================================\n");
	fclose(encInfo->fptr_secret);
	fclose(encInfo->fptr_src_image);
	fclose(encInfo->fptr_stego_image);
	return e_success;

}
