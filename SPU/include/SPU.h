#ifndef SPU_H
#define SPU_H 

#include <stdio.h>
#include <malloc.h>
#include "../../CPU_common.h"
#include "stack.h"
#include "colors.h"
#include "SPU_graph.h"
#include "SPU_fileHandling.h"

struct SPU
{
    Stack stack;
    int reg[AMOUNT_OF_REGISTERS];
    int* ram;
    uint8_t* curCommand;
    uint8_t* commands;
};

enum SPU_Error
{
    SPU_NO_ERROR,
    SPU_FILE_OPEN_ERROR,
    SPU_INCORRECT_INPUT,
    SPU_STACK_ERROR,
    SPU_NULL_SPU,
    SPU_NULL_ARRAY,
    SPU_MEM_ALLOC_ERROR,
    SPU_PARSE_ERROR,
};

SPU_Error execProgram(SPU* spu);

SPU_Error spuInit(SPU* spu, const char* inputFileName);

SPU_Error spuDtor(SPU* spu);

#endif