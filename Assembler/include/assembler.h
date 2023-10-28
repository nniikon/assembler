#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <assert.h>
#include "../../lib/parse.h"
#include "../../CPU_common.h"
#include "colors.h"
#include "ass_config.h"
#include "stringOperations.h"
#include "ass_err.h"

struct Label
{
    const char* name;
    size_t len;
    int adress;
};


struct Assembler
{
    Text inputText;
    const char* inputFileName;

    uint8_t* outputBuffer;
    size_t outputBufferPos;

    Label labels[MAX_NUMBER_OF_LABELS];
    size_t emptyLabel;

    AssErrorArray errorArray;
};


void assembly(Assembler* ass, FILE* outputFile);

AssemblerError assInit(Assembler* ass, const char* inputFile);

AssemblerError assDtor(Assembler* ass); // ASS DESTRUCTOR

#endif