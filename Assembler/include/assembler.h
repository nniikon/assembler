#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <assert.h>
#include "../../lib/parse.h"
#include "../../commands.h"

const int ASSEMBLER_POISON = INT_MIN;
#define ASSEMBLER_

enum AssemblerError
{
    ASSEMBLER_NO_ERROR,
    ASSEMBLER_NO_SUCH_COMMAND,
    ASSEMBLER_OPEN_FILE_ERROR,
};

AssemblerError textToAssembly(Text* txt, const char* outputFileName);

#endif