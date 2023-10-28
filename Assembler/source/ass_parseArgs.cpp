#include "../include/ass_parseArgs.h"

static void help()
{
    printf("-o\t[output file]\n"
           "-i\tinput file \n"
           "-h\tprint this message \n");
}

bool parseArguments(int argc, char** argv, const char** inFile, const char** outFile)
{
    const char invalidOptionErrorText[] = 
    "Invalid options or missing argument, use: %s -input <input_file> [-output] <output_file>\n";

    *inFile = NULL;
    const char* defaultOuputName = "assembly.bin";
    *outFile = defaultOuputName;

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
        else if (strcmp(argv[i], "-output") == 0 && i + 1 < argc) 
        {
            *outFile = argv[i + 1];
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
