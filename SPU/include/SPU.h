#ifndef SPU_H
#define SPU_H 

#include <stdio.h>
#include <malloc.h>
#include <inttypes.h>
#include "SPU_stack.h"
#include "../../lib/colors.h"
#include "SPU_dump.h"
#include "SPU_graph.h"
#include "SPU_fileHandling.h"
#include "../../common/CPU_common.h"


struct SPU_InitData
{
    const char* inputFileName;
    const char* dumpFileName;
    const char* extDumpFileName;
};

struct SPU
{
    Stack stack;
    int reg[AMOUNT_OF_REGISTERS];
    int* ram;
    uint8_t* curCommand;
    uint8_t* commands;
    uint8_t opcode;
    char* vramBuffer;
    SpuDumpInfo dump;
    bool isDump;
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
    SPU_FILE_ERROR,
    SPU_PARSE_ARGS_ERROR,
    SPU_DIVISION_BY_ZERO,
};

SPU_Error execProgram(SPU* spu);

SPU_Error spuInit(SPU* spu, SPU_InitData* args);

SPU_Error spuDtor(SPU* spu);

#endif