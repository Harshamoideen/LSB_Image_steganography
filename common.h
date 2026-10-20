#ifndef COMMON_H
#define COMMON_H

#include "types.h"

/* Magic string to identify whether stegged or not */
#define MAGIC_STRING "#*"

/* A 24-bit BMP has a 54 byte header; pixel data (where we hide bits) starts after it */
#define BMP_HEADER_SIZE 54

/* Check operation type 
Decide between encode (-e), decode (-d) or unsupported/help (anything else) */
OperationType check_operation_type(char *argv[]);

/* Print usage information */
void help_stegno(void);

void print_message(const char *message);

#endif
