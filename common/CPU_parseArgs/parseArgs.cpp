#include "parseArgs.h"

static void printOptName(StrArgument* arg)
{
    if (arg->isNecessary)
        printf(" %s ", arg->optName);
    else
        printf("[%s] ", arg->optName);
}


static void briefHelp(char** argv, ConsoleArgs* args)
{
    printf("Usage: %s ", argv[0]);
    for (size_t i = 0; i < args->nStrArgs; i++)
    {
        printOptName(args->argsArr + i);
        printf("<%s> ", args->argsArr[i].briefDescr);
    }
    printf("\n");
}


static void help(ConsoleArgs* args)
{
    for (size_t i = 0; i < args->nStrArgs; i++)
    {
        printOptName(args->argsArr + i);
        printf("\t <%s> \t - %s\n", args->argsArr[i].briefDescr, args->argsArr[i].fullDescr);
    }
}


bool parseArgs(int argc, char** argv, ConsoleArgs* args)
{
    assert(args);
    assert(argv);
    assert(args->argsArr);

    if (argc < 2)
    {
        printf(PARSE_INCORRECT_INPUT, argv[0], PARSE_OPT_HELP);
        briefHelp(argv, args);
        return false;
    }

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], PARSE_OPT_HELP) == 0)
        {
            help(args);
            return true;
        }
        for (size_t j = 0; j < args->nStrArgs; j++)
        {
            if (strcmp(argv[i], args->argsArr[j].optName) == 0)
            {
                i++;
                *(args->argsArr[j].output) = argv[i];
                continue;
            }
        }
    }

    for (size_t i = 0; i < args->nStrArgs; i++)
    {
        if (*(args->argsArr[i].output) == NULL && args->argsArr[i].isNecessary)
        {
            printf(PARSE_INCORRECT_INPUT, argv[0], PARSE_OPT_HELP);
            briefHelp(argv, args);
            return false;
        }
    }

    return true;
}
