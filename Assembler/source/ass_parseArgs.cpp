#include "../include/ass_parseArgs.h"

const char OPT_INPUT[] = "-i";
const char OPT_OUTPUT[] = "-o";
const char OPT_HELP[] = "-h";

static void help()
{
    printf("%s\t[output file]\n", OPT_OUTPUT);
    printf("%s\tinput file \n", OPT_INPUT);
    printf("%s\tprint this message \n", OPT_HELP);
}

static void printError(char** argv)
{
    const char invalidOptionErrorText[] = 
        "Invalid options or missing argument, use: %s %s <source_file> [%s] <output_file>\n";
    fprintf(stderr, invalidOptionErrorText, argv[0], OPT_INPUT, OPT_OUTPUT);
}

bool parseArguments(int argc, char** argv, const char** inFile, const char** outFile)
{
    *inFile = NULL;
    const char* defaultOuputName = "assembly.bin";
    *outFile = defaultOuputName;

    if (argc < 2) 
    {
        printError(argv);
        return false;
    }

    for (int i = 1; i < argc; i++) 
    {
        if (strcmp(argv[i], OPT_HELP) == 0)
        {
            help();
            return true;
        }
        else if (strcmp(argv[i], OPT_INPUT) == 0 && i + 1 < argc)
        {
            *inFile = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], OPT_OUTPUT) == 0 && i + 1 < argc) 
        {
            *outFile = argv[i + 1];
            i++;
        }
        else
        {
            printError(argv);
             return false;
        }
    }
    if (*inFile == NULL)
    {
        printError(argv);
        return false;
    }

    return true;
}
