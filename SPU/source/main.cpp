#include <stdio.h>
#include <stdlib.h>
#include "../include/SPU.h"
#include "../include/stack.h"

const char* SPU_INPUT_FILE_NAME = "../assembly.bin";

#define CHECK_STACK_ERROR(error)   \
do                                 \
{                                  \
    if ((error) != PARSE_NO_ERROR) \
    {                              \
        printf("error!\n");        \
        return error;              \
    }                              \
} while (0)                        \

#define CHECK_PARSE_ERROR(error)   \
do                                 \
{                                  \
    if ((error) != PARSE_NO_ERROR) \
    {                              \
        printf("parse error!\n");  \
        return error;              \
    }                              \
} while (0)                        \



int main()
{      
    uint8_t* buffer = NULL;
    size_t bufferSize = 0;
    SPU spu = {};


    ParseError parseError = getFileSize(SPU_INPUT_FILE_NAME, &bufferSize);
    CHECK_PARSE_ERROR(parseError);

    parseError = fileToIntBuffer(&buffer, bufferSize, SPU_INPUT_FILE_NAME);
    CHECK_PARSE_ERROR(parseError);
    

    spuInit(&spu, buffer);

    if (execProgram(&spu) == SPU_NO_ERROR)
    {
        fprintf(stderr, "NO ERROR OCCURRED\n");
    }
    stackDump(&spu.stack, STACK_NO_ERROR);
    spuDtor(&spu);


    free(buffer);
}