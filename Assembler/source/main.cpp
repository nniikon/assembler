#include "../include/ass_parseFile.h"
#include "../include/ass.h"
#include "../include/ass_stringOperations.h"
#include "../include/ass_parseArgs.h"
#include "../../common/CPU_parseArgs/parseArgs.h"


int main(int argc, char** argv)
{
    Assembler ass = {};
    AssemblerError assErr = ASSEMBLER_NO_ERROR;

    const char* inputFileName  = NULL;
    const char* outputFileName = DEFAULT_OUTPUT_FILE_NAME;

    StrArgument CONSOLE_ARGS_SGNTRS[] = 
    {
        {"-i",  "source_code",   "source code directory",      &inputFileName, true},
        {"-o",  "binary_file",   "executable file directory",  &outputFileName, false},
    };

    ConsoleArgs args = {sizeof(CONSOLE_ARGS_SGNTRS)/sizeof(CONSOLE_ARGS_SGNTRS[0]), CONSOLE_ARGS_SGNTRS};

    if (!parseArgs(argc, argv, &args))
        return ASSEMBLER_PARSE_ERROR;

    assErr = assInit(&ass, inputFileName, outputFileName);
    if (assErr != ASSEMBLER_NO_ERROR)
    {
        fprintf(stderr, "error initializing assembler.\n");
        return assErr;
    }

    assErr = assembly(&ass);
    if (assErr != ASSEMBLER_NO_ERROR)
    {
        assDtor(&ass);
        fprintf(stderr, "assembly error.\n");
        return assErr;
    }

    printAssError(&ass.errorArray, ass.inputText.line);

    assDtor(&ass);
    if (assErr != ASSEMBLER_NO_ERROR)
    {
        fprintf(stderr, "error destructing my ass.\n");
        return assErr;
    }
}