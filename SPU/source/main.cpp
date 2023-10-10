#include <stdio.h>
#include <stdlib.h>
#include "../include/SPU.h"
#include "../include/stack.h"

const char* SPU_INPUT_FILE_NAME = "assembly.bin";

#define CHECK_STACK_ERROR(error)   \
{                                  \
    if ((error) != NO_ERROR)       \
    {                              \
        printf("error!\n");        \
        return error;              \
    }                              \
}

#define CHECK_PARSE_ERROR(error)   \
{                                  \
    if ((error) != PARSE_NO_ERROR) \
    {                              \
        printf("parse error!\n");  \
        return error;              \
    }                              \
}

/*
TODO: rename config.h
Rename stack errors
*/
int main()
{      
    int i = 0;

    int* buffer = NULL;
    size_t bufferSize = 0;

    spuInit();

    ParseError parseError = getFileSize(SPU_INPUT_FILE_NAME, &bufferSize);
    CHECK_PARSE_ERROR(parseError);

    parseError = fileToIntBuffer(&buffer, bufferSize, SPU_INPUT_FILE_NAME);
    CHECK_PARSE_ERROR(parseError);
    
    execProgram(buffer, bufferSize);
    
    spuDtor();

    free(buffer);
}