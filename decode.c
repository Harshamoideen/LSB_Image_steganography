#include <stdio.h>
#include <string.h>
#include "types.h"
#include "common.h"
#include "decode.h"
#include "encode.h"

/*
 * Validate the command line for decoding:
 *   ./a.out -d <stego.bmp> [output_name]
 * Precondition: main() already checked that argv[2] exists.
 * The extension of 'output_name' is dropped - the real extension of the secret
 * file is stored inside the image and appended later by decode_file_extn().
 */
Status read_and_validate_encode_args_d(char *argv[], DecodeInfo *decInfo){
	/*source imgae must be a .bmp*/
	char *dot=strrchr(argv[2],'.');
	if(dot == NULL || strcmp(dot,".bmp") != 0)
	{
		printf("\nError: source image must be a .bmp file\n");
		return e_failure;

	}
	decInfo->stego_image_fname = argv[2];


	/*strtok to remove the extention*/
	if (argv[3] != NULL)
	{
		size_t len = strcspn(argv[3], ".");

		if (len >= sizeof(decInfo->output_fname))
		{
			printf("Error: output file name overflow.\n");
			return e_failure;
		}

		strncpy(decInfo->output_fname, argv[3], len);
		decInfo->output_fname[len] = '\0';
	}
	else
	{
		strcpy(decInfo->output_fname, "output");
	}

	return e_success;
}

/*
 * Open a file
 * Input: File name, file pointer address, and opening mode
 * Output: e_success if the file opens successfully, otherwise e_failure
 * Description: Open the specified file in the requested mode.
 *              Display an error message if the file cannot be opened.
 */
Status open_files1(char *filename, FILE **fptr, char *mode)
{
	// Src Image file
	*fptr = fopen(filename, mode);
	// Do Error handling
	if (*fptr == NULL)
	{
		perror("fopen");
		fprintf(stderr, "Error: Unable to open file %s\n", filename);

		return e_failure;
	}

	return e_success;
}
/*
 * Decode one byte from image LSBs
 * Input: Array of 8 image bytes
 * Output: Reconstructed character
 * Description: Extract the least significant bit from each image byte
 *              and combine the bits to reconstruct the original byte,
 *              starting from the least significant bit.
 */
char decode_lsb_to_byte(unsigned char data[])
{
	unsigned char lsb=0;
	for(int j=0;j<8;j++){
		lsb |= (unsigned char)((data[j] & 1) << j);
	}
	return lsb;

}

/*
 * Decode an integer from image LSBs
 * Input: Array of 32 image bytes
 * Output: Reconstructed unsigned integer
 * Description: Extract the least significant bit from each image byte
 *              and combine the 32 bits to reconstruct the original
 *              unsigned integer.
 */
unsigned int decode_lsb_to_int(unsigned char data[])
{
	unsigned int lsb=0;
	for(int i=0;i<32;i++){
		lsb |= (data[i] & 1) << i;
	}
	return lsb;

}

/*
 * Decode an integer from the stego image
 * Input: Pointer to the decoded integer and DecodeInfo structure pointer
 * Output: e_success if 32 image bytes are read and decoded,
 *         otherwise e_failure
 * Description: Read 32 bytes from the stego image and extract their
 *              least significant bits to reconstruct an integer.
 */
Status decode_int_from_image(unsigned int *lsb, DecodeInfo *decInfo)
{
	unsigned char buf[32];
	if(fread(buf,1,32,decInfo->fptr_stego_image) != 32)
		return e_failure;
	*lsb = decode_lsb_to_int(buf);
	return e_success;

}
/*
 * Decode 'size' characters into 'data' and add a '\0'.
 * The caller must provide a buffer of at least size + 1 bytes.
 */
Status decode_data_from_image(char *data,size_t size,DecodeInfo *decInfo)
{
	unsigned char buf[8];
	for(size_t i=0;i<size;i++){
		if(fread(buf,1,8,decInfo->fptr_stego_image)!=8)
			return e_failure;
		data[i]=(char)decode_lsb_to_byte(buf);
	}
	data[size]='\0';

	return e_success;
}
/*
 * Decode the file extension (length was already read) and append it to the
 * output file name.
 */
Status decode_file_extn(unsigned int extn_len, DecodeInfo *decInfo){
	if (extn_len + 1 > sizeof(decInfo->extn_secret_file))
	{
		printf("Error: extension length %u is invalid\n", extn_len);
		return e_failure;
	}

	if (decode_data_from_image(decInfo->extn_secret_file,extn_len, decInfo) == e_failure)
	{
		return e_failure;
	}
	strcat(decInfo->output_fname,".");
	strcat(decInfo->output_fname,decInfo->extn_secret_file);
	return e_success;
}
/*
 * Skip the BMP header and check that the magic string is there.
 * If it is missing the image was never encoded by this program.
 */
Status decode_magic_string(char *magic_string,DecodeInfo *decInfo)
{
	fseek(decInfo->fptr_stego_image,BMP_HEADER_SIZE,SEEK_SET);
	size_t len =strlen(MAGIC_STRING);
	char decoded[len + 1];
	if(decode_data_from_image(decoded,strlen(MAGIC_STRING),decInfo) == e_failure)
		return e_failure;
	if(strcmp(decoded,magic_string) !=0){
		return e_failure;
	}

	return e_success;

}

/* Decode 'len' bytes and write them straight into the output file */
Status decode_secret_data_from_image(unsigned int len, DecodeInfo *decInfo)
{
	unsigned char buf[8];
	char data;
	for (unsigned int i = 0; i < len; i++)
	{
		if (fread(buf, 1, 8, decInfo->fptr_stego_image) != 8)
			return e_failure;
		data = decode_lsb_to_byte(buf);
		if (fwrite(&data, 1, 1, decInfo->fptr_output) != 1)
			return e_failure;
	}
	return e_success;
}

/*
 * Perform the complete decoding process
 * Input: DecodeInfo structure pointer
 * Output: e_success if decoding completes successfully,
 *         otherwise e_failure
 * Description: Open the stego image, validate the magic string,
 *              decode the secret file extension and file size,
 *              create the output file, and extract the secret data.
 *              Display appropriate error messages if any step fails.
 */
Status do_decoding(DecodeInfo *decInfo)
{
	print_message("INFO: Opening stego image...");
	if( open_files1(decInfo->stego_image_fname, &decInfo->fptr_stego_image,"r") == e_failure)
	{
		printf("fopen failure\n");
		return e_failure;
	}
	print_message("INFO: Stego image  opened successfully.\n");
	print_message("INFO: Checking stego image signature...");
	if(decode_magic_string(MAGIC_STRING,decInfo) == e_failure)
	{
		printf("Error: Invalid stego image or magic string mismatch.\n");
		return e_failure;
	}
	print_message("INFO: Image signature verified.\n");
	print_message("INFO: Decoding secret file extension's size...");
	unsigned int extn_len=0;
	if(decode_int_from_image(&extn_len,decInfo) == e_failure)
	{
		printf("Error: Unable to decode the secret file extension length.\n");
		return e_failure;
	}
	print_message("INFO: Decoding secret file extension...");

	if(decode_file_extn(extn_len,decInfo) == e_failure)
	{
		printf("Error: Unable to decode the secret file extension.\n");
		return e_failure;
	}

	print_message("INFO: Opening output file...");
	if( open_files1(decInfo->output_fname,&decInfo->fptr_output,"w") == e_failure){
		printf("Error: Unable to create the output file.\n");
		return e_failure;
	}
	print_message("INFO: Output file opened successfully.\n");

	print_message("INFO: Decoding secret file size...");
	unsigned int secret_size=0;
	if(decode_int_from_image(&secret_size,decInfo) == e_failure)
	{
		printf("Error: Unable to decode the secret file size.\n");
		return e_failure;
	}
	printf("Decoded secret size = %u bytes\n", secret_size);
	print_message("INFO: Decoding secret file data...");
	if(decode_secret_data_from_image(secret_size,decInfo) == e_failure)
	{
		printf("Error: Failed to decode the complete secret file.\n");
		return e_failure;
	}
	printf("\n===============================================\n");
	printf("        Decoding completed successfully!\n");
	printf("===============================================\n");
	fclose(decInfo->fptr_output);
	fclose(decInfo->fptr_stego_image);
	return e_success;
}
