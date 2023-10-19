#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <assert.h>
#include "../../lib/parse.h"
#include "../../CPU_common.h"
#include "colors.h"

const int ASSEMBLER_POISON = INT_MIN;

struct Assembler
{
    Text inputText;
    const char* inputFileName;
    int* outputBuffer;
    size_t outputBufferPos;
};

enum AssemblerError
{
    ASSEMBLER_NO_ERROR,
    ASSEMBLER_NO_SUCH_COMMAND,
    ASSEMBLER_OPEN_FILE_ERROR,
    ASSEMBLER_PARSE_ERROR,
    ASSEMBLER_ALLOCATION_ERROR,
};

AssemblerError textToAssembly(Assembler* ass, const char* outputFileName);

AssemblerError AssInit(Assembler* ass, const char* inputFile);

AssemblerError AssDtor(Assembler* ass); // ASS DESTRUCTOR

#endif