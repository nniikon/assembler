#ifndef SPU_FILE_HANDLING_H
#define SPU_FILE_HANDLING_H

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>

enum SPU_fileError
{
    SPU_FILE_NO_ERROR,
    SPU_FILE_FILE_OPEN_ERROR,
    SPU_FILE_MEM_ALLOCATION_ERROR,
    SPU_FILE_STAT_ERROR,
    SPU_FILE_INVALID_ARGS,
    SPU_FILE_UNEXPECTED_ERROR,
    SPU_FILE_FREAD_ERROR,
};

SPU_fileError createBinaryBuffer(uint8_t** buffer, const char* FILE_NAME);

#endif