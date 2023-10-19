#ifndef SPU_H
#define SPU_H 

#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <malloc.h>
#include "../../CPU_common.h"
#include "stack.h"

struct SPU
{
    Stack stack;
    int reg[AMOUNT_OF_REGISTERS];
    int* curCommand;
};


enum SPU_Error
{
    SPU_NO_ERROR,
    SPU_FILE_OPEN_ERROR,
    SPU_INCORRECT_INPUT,
    SPU_STACK_ERROR,
    SPU_NULL_SPU,
    SPU_NULL_ARRAY,
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

SPU_Error execProgram(SPU* spu);

SPU_Error spuInit(SPU* spu, int* commandsArr);

SPU_Error spuDtor(SPU* spu);

#endif