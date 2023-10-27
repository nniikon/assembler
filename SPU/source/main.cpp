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
    SPU_Error err = SPU_NO_ERROR;
    ParseError parseError = PARSE_NO_ERROR;

    parseError = getFileSize(SPU_INPUT_FILE_NAME, &bufferSize);
    CHECK_PARSE_ERROR(parseError);

    parseError = fileToIntBuffer(&buffer, bufferSize, SPU_INPUT_FILE_NAME);
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