#include "../../lib/parse.h"
#include "../include/assembler.h"

const char* INPUT_FILE_NAME = "source_code";
const char* OUTPUT_FILE_NAME = "assembly.bin";


int main()
{
    Text txt = {};
    ParseError error = textInit(INPUT_FILE_NAME, &txt);

    if (error != PARSE_NO_ERROR)
    {
        fprintf(stderr, "ERROR PARSING BUFFER");
    }
    textToAssembly(&txt, OUTPUT_FILE_NAME);

    textDtor(&txt);
}