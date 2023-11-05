#ifndef ASS_ERR_H
#define ASS_ERR_H
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ass_config.h"
#include "ass_parseFile.h"

enum CommandError 
{
    // Generate enum.
    #define DEF_ERR(name, errStr) CMD_ ## name,
    #include "../include/ass_errors_codegen.h"
    #undef DEF_ERR
};

enum AssemblerError
{
    ASSEMBLER_NO_ERROR,
    ASSEMBLER_NO_SUCH_COMMAND,
    ASSEMBLER_OPEN_FILE_ERROR,
    ASSEMBLER_PARSE_ERROR,
    ASSEMBLER_ALLOCATION_ERROR,
    ASSEMBLER_FWRITE_ERROR,
    ASSEMBLER_WRONG_ARGUMENTS,
};

struct AssError
{
    CommandError err;
    size_t line;
    const char* fileName;
};

struct AssErrorArray
{
    AssError* err;
    size_t emptyIndex;
    size_t capacity;
};

AssemblerError pushAssErrArray(AssErrorArray* errArr, AssError* error);

AssemblerError AssErrorInit(AssErrorArray* errArr);

void AssErrorDtor(AssErrorArray* errArr);

void printAssError(AssErrorArray* errArr, const Line* line);

#endif