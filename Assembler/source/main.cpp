#include "../../lib/parse.h"
#include "../include/assembler.h"

const char* INPUT_FILE_NAME = "source_code";
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

int main()
{
    Text txt = {};
    CHECK_PARSING_ERROR  (     textInit(INPUT_FILE_NAME, &txt)            );
    CHECK_ASSEMBLER_ERROR(     textToAssembly(&txt, OUTPUT_FILE_NAME)     );

    textDtor(&txt);
}