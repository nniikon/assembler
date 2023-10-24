#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <assert.h>
#include "../../lib/parse.h"
#include "../../CPU_common.h"
#include "colors.h"
#include "ass_config.h"
#include "stringOperations.h"

struct Label
{
    char* name;
    size_t len;
    int adress;
};


enum CommandError 
{
    // Generate enum.
    #define DEF_ERR(name, errStr) CMD_ ## name,
    #include "../include/errors.h"
    #undef DEF_ERR
};


struct AssError
{
    CommandError err;
    size_t line;
};


struct AssErrorArray
{
    AssError* err;
    size_t emptyIndex;
    size_t capacity;
};


struct Assembler
{
    Text inputText;
    const char* inputFileName;
    
    u_int8_t* outputBuffer;
    size_t outputBufferPos;

    Label labels[MAX_NUMBER_OF_LABELS];
    size_t emptyLabel;

    AssErrorArray errorArray;
};

enum AssemblerError
{
    ASSEMBLER_NO_ERROR,
    ASSEMBLER_NO_SUCH_COMMAND,
    ASSEMBLER_OPEN_FILE_ERROR,
    ASSEMBLER_PARSE_ERROR,
    ASSEMBLER_ALLOCATION_ERROR,
};

void assembly(Assembler* ass, FILE* outputFile);

AssemblerError AssInit(Assembler* ass, const char* inputFile);

AssemblerError AssDtor(Assembler* ass); // ASS DESTRUCTOR

#endif