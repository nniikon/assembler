#include "../include/ass_err.h"


static const char* errorStr[] = 
{
    // Generate error string names.
    #define DEF_ERR(name, errStr) errStr,
    #include "../include/errors.h"
    #undef DEF_ERR
};

#define ASSEMBLER_DEBUG

#ifdef ASSEMBLER_DEBUG
    #define DUMP_PRINT(...)\
    do\
    {\
        fprintf(stderr, "file: %s line: %d\t\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
    } while (0)

    #define DUMP_PRINT_CYAN(...)\
    do\
    {\
        fprintf(stderr, CYAN "file: %s line: %d\t\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
        fprintf(stderr, RESET);\
    } while (0)
#else
    #define DUMP_PRINT(...) do {} while(0)
    #define DUMP_PRINT_CYAN(txt) do {} while(0) 
#endif

AssemblerError AssErrorInit(AssErrorArray* errArr)
{
    assert(errArr);

    AssError* temp = (AssError*) calloc(DEFAULT_SIZE_OF_ERROR_ARR, sizeof(AssError));
    if (temp == NULL)
    {
        DUMP_PRINT("Error allocating memory\n");
        return ASSEMBLER_ALLOCATION_ERROR;
    }

    errArr->err = temp;
    errArr->capacity = DEFAULT_SIZE_OF_ERROR_ARR;
    errArr->emptyIndex = 0;

    return ASSEMBLER_NO_ERROR;
}


void AssErrorDtor(AssErrorArray* errArr)
{
    free(errArr->err);
}


void printAssError(AssErrorArray* errArr, const Line* line)
{

    for (size_t errorID = 0; errorID < errArr->emptyIndex; errorID++)
    {
        AssError cmd = errArr->err[errorID];

        fprintf(stderr, "%s(%zu): " MAGENTA "%s" RESET "\n", errArr->err[errorID].fileName, cmd.line + 1, errorStr[(int)cmd.err]);       
        fprintf(stderr, "\t %zu| %s\n\n", cmd.line + 1, line[cmd.line].str);
    }
}


AssemblerError pushAssErrArray(AssErrorArray* errArr, AssError* error)
{
    const size_t CAPACITY_MULTIPLIER = 2;

    DUMP_PRINT_CYAN("Pushing error\n");
    if ((errArr->emptyIndex + 1) == errArr->capacity)
    {
        AssError* temp = (AssError*) realloc(errArr->err, errArr->capacity * CAPACITY_MULTIPLIER * sizeof(AssError));
        if (temp == NULL)
        {
            DUMP_PRINT("Error allocating memory\n");
            return ASSEMBLER_ALLOCATION_ERROR;
        }
        errArr->err = temp; 
        errArr->capacity *= CAPACITY_MULTIPLIER;
    }

    errArr->err[errArr->emptyIndex].err = error->err;
    errArr->err[errArr->emptyIndex].line = error->line;
    errArr->emptyIndex++;
    DUMP_PRINT_CYAN("Pushing error success\n");

    return ASSEMBLER_NO_ERROR;
}