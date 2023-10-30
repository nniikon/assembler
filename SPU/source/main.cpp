#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/SPU.h"
#include "../include/stack.h"
#include "../include/SPU_parseArgs.h"

#define CHECK_PARSE_ERROR(error)          \
do                                        \
{                                         \
    if ((error) != PARSE_NO_ERROR)        \
    {                                     \
        fprintf(stderr, "parse error!\n");\
        return error;                     \
    }                                     \
} while (0)                               \



int main(int argc, char** argv)
{
    const char* inputFileName = NULL;
    if (!parseArguments(argc, argv, &inputFileName))
        return SPU_PARSE_ARGS_ERROR;

    SPU spu = {};
    SPU_Error err = SPU_NO_ERROR;

    spuInit(&spu, inputFileName);

    err = execProgram(&spu); 
    if (err != SPU_NO_ERROR)
    {
        spuDtor(&spu);
        fprintf(stderr, "ERROR OCCURRED\n");
        return err;
    }
    spuDtor(&spu);
}
