#include "../include/ass_parseFile.h"
#include "../include/ass.h"
#include "../include/ass_stringOperations.h"
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

    assErr = assInit(&ass, inputFileName, outputFileName);
    if (assErr != ASSEMBLER_NO_ERROR)
    {
        fprintf(stdout, "error initializing assembler.\n");
        return assErr;
    }

    assErr = assembly(&ass);
    if (assErr != ASSEMBLER_NO_ERROR)
    {
        assDtor(&ass);
        fprintf(stdout, "assembly error.\n");
        return assErr;
    }

    printAssError(&ass.errorArray, ass.inputText.line);

    assDtor(&ass);
    if (assErr != ASSEMBLER_NO_ERROR)
    {
        fprintf(stdout, "error destructing my ass.\n");
        return assErr;
    }
}