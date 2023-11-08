#ifndef SPU_PARSE_ARGS_H
#define SPU_PARSE_ARGS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "parseArgs_config.h"

struct StrArgument
{
    const char* optName; 
    const char* briefDescr;
    const char* fullDescr;
    const char** output;
    bool isNecessary;
};


struct ConsoleArgs
{
    size_t nStrArgs;
    StrArgument *argsArr;
};

bool parseArgs(int argc, char** argv, ConsoleArgs* args);

#endif