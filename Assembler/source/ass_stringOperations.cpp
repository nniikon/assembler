#include "../include/ass_stringOperations.h"


// char getWordSize1(size_t* size, const char* str, const char* delim)
// {
//     assert(size);
//     assert(str);
//     assert(delim);

//     *size = strcspn(str, delim);
//     return str[*size];
// }

//0x50:  ptr = 0x0

//0x100: "Hello"

// void moveToNextWord1(const char** input, size_t size, const char* delim)
// {
//     assert(input);
//     assert(delim);

//     *input += size;

//     size_t shift = strspn(*input, delim);
//     *input += shift;

//     if (*input[0] == 0)
//         (*input) = NULL;
// }


size_t nChrInLine(const char* str, const char chr)
{
    size_t count = 0;

    str = strchr(str, chr);
    while (str != NULL) 
    {
        str++;
        count++;
        str = strchr(str, chr);
    }
    return count;
}


const char* skipDelims(const char* input, const char* delims)
{
    assert(input);
    assert(delims);

    size_t shift = strspn(input, delims);
    input += shift;

    return input;
}


const char* skipWhiteSpaces(const char* input)
{
    assert(input);

    size_t i = 0;
    for (i = 0; input[i] != '\0'; i++)
    {
        if (!isspace(input[i]))
            return (input + i);
    }
    return input + i;
}


size_t getWordSize(const char* input, const char* delims)
{
    assert(input);

    size_t shift = strcspn(input, delims);

    return shift;
}