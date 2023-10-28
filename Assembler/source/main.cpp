#include "../../lib/parse.h"
#include "../include/assembler.h"
#include "../include/stringOperations.h"
#include "../include/ass_parseArgs.h"


int main(int argc, char** argv)
{
    Assembler ass = {};

    AssemblerError assErr = ASSEMBLER_NO_ERROR;


    const char* inputFileName  = NULL;
    const char* outputFileName = NULL;

    if (parseArguments(argc, argv, &inputFileName, &outputFileName) == false)
    {
        return ASSEMBLER_WRONG_ARGUMENTS;
    }

    // Open the output file.
    FILE* outputFile = fopen(outputFileName, "wb");
    if (outputFile == NULL)
    {
        fprintf(stderr, "failed to open %s\n", outputFileName);
        return ASSEMBLER_OPEN_FILE_ERROR;
    }

    assErr = assInit(&ass, inputFileName);
    if (assErr != ASSEMBLER_NO_ERROR)
    {
        fclose(outputFile);
        fprintf(stderr, "error initializing assembler.\n");
        return assErr;
    }

    assembly(&ass, outputFile);
    fclose(outputFile);

    printAssError(&ass.errorArray, ass.inputText.line);

    assDtor(&ass);
    if (assErr != ASSEMBLER_NO_ERROR)
    {
        fprintf(stderr, "error destructing my ass.\n");
        return assErr;
    }
}