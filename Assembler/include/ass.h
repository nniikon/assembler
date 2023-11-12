#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <assert.h>
#include "ass_parseFile.h"
#include "../../common/CPU_common.h"
#include "ass_config.h"
#include "ass_stringOperations.h"
#include "ass_err.h"
#include "ass_listing.h"

struct AssCommand
{
    size_t nRegs;
    size_t nNums;
    size_t nLabels;
    size_t nMems;

    uint8_t opcode;
    uint8_t reg;
    int imm;

    size_t cmdID;

    size_t line;

    CommandError error;
};

struct AssemblerInitInfo
{
    const char* inputFileName;
    const char* outputFileName;
    const char* listingFileName;
};

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
    FILE* listingFile;

    Text inputText;
    const char* inputFileName;

    uint8_t* outputBuffer;
    size_t outputBufferPos;

    LabelArr labelArr;

    AssErrorArray errorArray;
};

AssemblerError assembly(Assembler* ass);

AssemblerError assInit(Assembler* ass, const AssemblerInitInfo* info);

AssemblerError assDtor(Assembler* ass); // ASS DESTRUCTOR!!!!!!!!!!!!

#endif