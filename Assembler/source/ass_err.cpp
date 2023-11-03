#include "../include/ass_err.h"

#define DUMP_DEBUG
#include "../../lib/dump.h"

static const char* errorStr[] = 
{
    // Generate error string names.
    #define DEF_ERR(name, errStr) errStr,
    #include "../include/errors.h"
    #undef DEF_ERR
};


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

        fprintf(stderr, "%s", errArr->err[errorID].fileName);
        fprintf(stderr, "(%zu): " MAGENTA "%s" RESET "\n", cmd.line + 1, errorStr[(int)cmd.err]);
        fprintf(stderr, "\t %zu| %s\n\n", cmd.line + 1, line[cmd.line].str);
    }
}


AssemblerError pushAssErrArray(AssErrorArray* errArr, AssError* error)
{
    const size_t CAPACITY_MULTIPLIER = 2;

    DUMP_PRINT_CYAN("Pushing error\n");
    DUMP_PRINT("emptyIndex: %zu\n", errArr->emptyIndex);
    DUMP_PRINT("capacity: %zu\n", errArr->capacity);
    if ((errArr->emptyIndex + 1) >= errArr->capacity)
    {
        DUMP_PRINT_CYAN("Increasing the capacity of the error array.\n");
        AssError* temp = (AssError*) realloc(errArr->err, errArr->capacity * CAPACITY_MULTIPLIER * sizeof(AssError));
        if (temp == NULL)
        {
            DUMP_PRINT("Error allocating memory\n");
            return ASSEMBLER_ALLOCATION_ERROR;
        }
        errArr->err = temp; 
        DUMP_PRINT_CYAN("Old capacity: %zu\n", errArr->capacity);
        errArr->capacity *= CAPACITY_MULTIPLIER;
        DUMP_PRINT_CYAN("New capacity: %zu\n", errArr->capacity);

    }

    errArr->err[errArr->emptyIndex].err = error->err;
    errArr->err[errArr->emptyIndex].line = error->line;
    errArr->err[errArr->emptyIndex].fileName = error->fileName;
    errArr->emptyIndex++;
    DUMP_PRINT_CYAN("Pushing error success\n");

    return ASSEMBLER_NO_ERROR;
}