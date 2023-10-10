#ifndef SPU_H
#define SPU_H 

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <malloc.h>
#include "../../commands.h"
#include "stack.h"


const int FLOATING_POINTER_COEFFICIENT = 100;

enum SPU_Error
{
    SPU_NO_ERROR,
    SPU_FILE_OPEN_ERROR,
    SPU_INCORRECT_INPUT,
    SPU_STACK_ERROR,
};

enum ParseError
{
    PARSE_NO_ERROR,
    PARSE_FILE_OPEN_ERROR,
    PARSE_MEM_ALLOCATION_ERROR,
    PARSE_STAT_ERROR,
    PARSE_INVALID_ARGS,
    PARSE_UNEXPECTED_ERROR,
    PARSE_FREAD_ERROR,
};

ParseError getFileSize(const char* fileName, size_t* size);

ParseError fileToIntBuffer(int** buffer, const size_t size, const char* FILE_NAME);

SPU_Error execProgram(int* buffer, size_t size);

SPU_Error spuInit();

SPU_Error spuDtor();

#endif