#include "../../lib/parse.h"
#include "../include/assembler.h"
#include "../include/stringOperations.h"

const char* INPUT_FILE_NAME = "../source_code";
const char* OUTPUT_FILE_NAME = "../assembly.bin";

#define CHECK_PARSING_ERROR(...)\
do\
{\
    ParseError error_deb = __VA_ARGS__;\
    if (error_deb != PARSE_NO_ERROR)\
    {\
        fprintf(stderr, "ERROR PARSING BUFFER\n");\
        assert(0);\
    }\
} while (0)


#define CHECK_ASSEMBLER_ERROR(...)\
do\
{\
    AssemblerError error_deb = __VA_ARGS__;\
    if (error_deb != ASSEMBLER_NO_ERROR)\
    {\
        fprintf(stderr, "ERROR ASSEMBLING\n");\
        assert(0);\
    }\
} while (0)


static void help();

static bool parseArguments(int argc, char** argv, const char** inFile, const char** outFile);


int main(int argc, char** argv)
{
    Assembler ass = {};

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

    AssInit(&ass, inputFileName);
    assembly(&ass, outputFile);
    printAssError(&ass.errorArray, ass.inputText.line);
    AssDtor(&ass);
}


static bool parseArguments(int argc, char** argv, const char** inFile, const char** outFile)
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

    // fprintf(stderr, "output: %s\n", *outFile);
    // fprintf(stderr, "infile: %s\n", *inFile);

    return true;
}


static void help()
{
    printf("-o\t[output file]\n"
           "-i\tinput file \n"
           "-h\tprint this message \n");
}