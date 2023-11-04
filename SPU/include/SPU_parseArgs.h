#ifndef SPU_PARSE_ARGS_H
#define SPU_PARSE_ARGS_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct ConsoleArgs
{
    const char* inFile;
    const char* dumpFile;
    bool genDump;
    bool genExtDump;
};

bool parseArguments(int argc, char** argv, ConsoleArgs* args);

#endif