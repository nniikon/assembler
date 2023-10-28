#include <stdio.h>
#include <stdlib.h>
#include "../include/SPU.h"
#include "../include/stack.h"
#include <string.h>

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

static bool parseArguments(int argc, char** argv, const char** inFile);

static void help();

int main(int argc, char** argv)
{
    const char* inputFileName = NULL;
    uint8_t* buffer = NULL;
    size_t bufferSize = 0;
    SPU spu = {};
    SPU_Error err = SPU_NO_ERROR;
    ParseError parseError = PARSE_NO_ERROR;

    parseArguments(argc, argv, &inputFileName);

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


static bool parseArguments(int argc, char** argv, const char** inFile)
{
    const char invalidOptionErrorText[] = 
    "Invalid options or missing argument, use: %s -input <input_file>\n";

    *inFile = NULL;

    if (argc < 2) 
    {
        fprintf(stderr, invalidOptionErrorText, argv[0]);
        return false;
    }

    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], "-h") == 0)
        {
            help();
            return true;
        }
        else if (strcmp(argv[i], "-input") == 0 && i + 1 < argc)
        {
            *inFile = argv[i + 1];
            i++; // Skip the next argument
        }
        else
        {
            fprintf(stderr, invalidOptionErrorText, argv[0]);
            return false;
        }
    }
    if (*inFile == NULL)
    {
        fprintf(stderr, invalidOptionErrorText, argv[0]);
        return false;
    }

    return true;
}


static void help()
{
    printf("-input\tinput file \n");
}

