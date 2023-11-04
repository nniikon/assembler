#include "../include/SPU_parseArgs.h"

#define DUMP_DEBUG
#include "../../lib/dump.h"

const char OPT_INPUT[] = "-i";
const char OPT_DUMP[] = "-d";
const char OPT_EXT_DUMP[] = "-da";
const char OPT_HELP[] = "-h";

static void help()
{
    printf("Usage: SPU -i <source file> [-d][-da] <dump file>\n");
    printf("Options:\n");
    printf("%s       <source file>     source file\n", OPT_INPUT);
    printf("%s, %s  <dump file>       generate dump file: %s = simple dump, %s = extended dump\n",
            OPT_DUMP, OPT_EXT_DUMP, OPT_DUMP, OPT_EXT_DUMP);
}

bool parseArguments(int argc, char** argv, ConsoleArgs* args)
{
    assert(argv);
    assert(args);

    DUMP_PRINT("parseArguments() started\n");

    const char invalidOptionErrorText[] = 
        "Invalid options or missing argument, use: %s -h for help\n";

    args->inFile = NULL;
    args->genDump = false;
    args->genExtDump = false;
    args->dumpFile = NULL;

    if (argc < 3)
    {
        fprintf(stderr, invalidOptionErrorText, argv[0]);
        return false;
    }

    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], OPT_HELP) == 0)
        {
            help();
            return false;
        }
        else if (i + 1 < argc)
        {
            if (strcmp(argv[i], OPT_INPUT) == 0)
            {
                args->inFile = argv[i + 1];
                i++;
            }
            else if (strcmp(argv[i], OPT_DUMP) == 0)
            {
                args->genDump = true;
                args->dumpFile = argv[i + 1];
                i++;
            }
            else if (strcmp(argv[i], OPT_EXT_DUMP) == 0)
            {
                args->genExtDump = true;
                args->dumpFile = argv[i + 1];
                i++;
            }
        }
        else
        {
            fprintf(stderr, invalidOptionErrorText, argv[0]);
            return false;
        }
    }
    if (args->inFile == NULL)
    {
        fprintf(stderr, invalidOptionErrorText, argv[0]);
        return false;
    }

    DUMP_PRINT("ParseArguments() ended successfully:\n");
    DUMP_PRINT("inFile: %s\n", args->inFile);
    DUMP_PRINT("genLising: %d\n", args->genDump);
    DUMP_PRINT("genExtLising: %d\n", args->genExtDump);
    if (args->dumpFile != NULL)
        DUMP_PRINT("dumpFile: %s\n", args->dumpFile);

    return true;
}