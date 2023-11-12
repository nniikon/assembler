#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/SPU.h"
#include "../include/SPU_stack.h"

#include "../../common/CPU_parseArgs/parseArgs.h"

int main(int argc, char** argv)
{
    SPU_InitData data = {};

    StrArgument CONSOLE_ARGS_SGNTR[] = 
    {
        {"-i",  "source_file", "source file directory",        &data.inputFileName,   true},
        {"-d",  "dump_file",   "dump file directory",          &data.dumpFileName,    false},
        {"-da", "dump_file",   "extended dump file directory", &data.extDumpFileName, false},
    };

    ConsoleArgs consoleArgs = {sizeof(CONSOLE_ARGS_SGNTR) / sizeof(CONSOLE_ARGS_SGNTR[0]), CONSOLE_ARGS_SGNTR};

    if (!parseArgs(argc, argv, &consoleArgs))
        return SPU_PARSE_ARGS_ERROR;

    SPU spu = {};
    SPU_Error err = SPU_NO_ERROR;

    err = spuInit(&spu, &data);
    if (err != SPU_NO_ERROR)
    {
        fprintf(stderr, "ERROR OCCURED\n");
        return err;
    }

    err = execProgram(&spu); 
    if (err != SPU_NO_ERROR)
    {
        spuDtor(&spu);
        fprintf(stderr, "ERROR OCCURRED\n");
        return err;
    }

    err = spuDtor(&spu);
    if (err != SPU_NO_ERROR)
    {
        fprintf(stderr, "ERROR OCCURRED\n");
        return err;
    }
}
