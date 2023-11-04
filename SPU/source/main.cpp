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
    ConsoleArgs consoleArgs = {};
    if (!parseArguments(argc, argv, &consoleArgs))
        return SPU_PARSE_ARGS_ERROR;

    SPU spu = {};
    SPU_Error err = SPU_NO_ERROR;

    err = spuInit(&spu, &consoleArgs);
    if (err != SPU_NO_ERROR)
    {
        fprintf(stdout, "ERROR OCCURED\n");
        return err;
    }

    err = execProgram(&spu); 
    if (err != SPU_NO_ERROR)
    {
        spuDtor(&spu);
        fprintf(stdout, "ERROR OCCURRED\n");
        return err;
    }

    err = spuDtor(&spu);
    if (err != SPU_NO_ERROR)
    {
        fprintf(stdout, "ERROR OCCURRED\n");
        return err;
    }
}
