#include "../include/stringOperations.h"

char getWordSize(size_t* size, const char* str, const char* delim)
{
    assert(size);
    assert(str);
    assert(delim);

    *size = strcspn(str, delim);
    return str[*size];
}


void moveToNextWord(const char** input, size_t size, const char* delim)
{
    assert(input);
    assert(delim);

    *input += size;

    size_t shift = strspn(*input, delim);
    *input += shift;

    if (*input[0] == 0)
        (*input) = NULL;
}


bool chrAppearsMoreTimes(const char* str, const char chr, size_t amount)
{
    size_t count = 0;

    str = strchr(str, chr);
    while (str != NULL) 
    {
        str++;
        count++;
        if (count > amount)
            return true;

        str = strchr(str, chr);
    }
    return false;
}