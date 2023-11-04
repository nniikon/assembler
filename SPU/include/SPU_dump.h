#ifndef SPU_DUMP_H
#define SPU_DUMP_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include "stack.h"
#include "../../CPU_common.h"
#include "SPU_config.h"


struct CmdInfo
{
    bool hasReg;
    bool hasImm;
    bool hasMem;
    int regID;
    int regValue;
    int imm;
    int mem;
};

struct SpuDumpInfo
{
    FILE* file;
    size_t adress;
    uint8_t* startAdressPtr;
    size_t shift;
    const char* cmdName;
    CmdInfo cmdInfo;
    bool isExtended;

    bool isResValue;
};

void dumpSpu(SpuDumpInfo* dump, int* regs, int* ram, Stack* stk);


#endif