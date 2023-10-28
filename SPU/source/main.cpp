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
    parseArguments(argc, argv, &inputFileName);

    SPU spu = {};
    SPU_Error err = SPU_NO_ERROR;
    ParseError parseError = PARSE_NO_ERROR;


    uint8_t* buffer = NULL;
    size_t bufferSize = 0;
    parseError = getFileSize(inputFileName, &bufferSize);
    CHECK_PARSE_ERROR(parseError);

    parseError = fileToIntBuffer(&buffer, bufferSize, inputFileName);
    CHECK_PARSE_ERROR(parseError);


    spuInit(&spu, buffer);

    err = execProgram(&spu); 
    if (err != SPU_NO_ERROR)
    {
        free(buffer);
        spuDtor(&spu);
        fprintf(stderr, "ERROR OCCURRED\n");
        return err;
    }
    spuDtor(&spu);
    free(buffer);
}
