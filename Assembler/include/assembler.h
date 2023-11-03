#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <assert.h>
#include "../../lib/parse.h"
#include "../../CPU_common.h"
#include "ass_config.h"
#include "stringOperations.h"
#include "ass_err.h"

struct Label
{
    const char* name;
    size_t len;
    int adress;
};


struct LabelArr
{
    Label* labels;
    size_t emptyLabel;
    size_t nLabels;
};


struct Assembler
{
    Text inputText;
    const char* inputFileName;

    uint8_t* outputBuffer;
    size_t outputBufferPos;

    LabelArr labelArr;

    AssErrorArray errorArray;
};


void assembly(Assembler* ass, FILE* outputFile);

AssemblerError assInit(Assembler* ass, const char* inputFile);

AssemblerError assDtor(Assembler* ass); // ASS DESTRUCTOR

#endif