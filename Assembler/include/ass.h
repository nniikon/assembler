#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <assert.h>
#include "ass_parseFile.h"
#include "../../common/CPU_common.h"
#include "ass_config.h"
#include "ass_stringOperations.h"
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
    FILE* outputFile;

    Text inputText;
    const char* inputFileName;

    uint8_t* outputBuffer;
    size_t outputBufferPos;

    LabelArr labelArr;

    AssErrorArray errorArray;
};

AssemblerError assembly(Assembler* ass);

AssemblerError assInit(Assembler* ass, const char* inputFileName, const char* outputFileName);

AssemblerError assDtor(Assembler* ass);

#endif