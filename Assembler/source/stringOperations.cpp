#include "../include/stringOperations.h"

char getWordSize(size_t* size, const char* str, const char* delim)
{
    assert(size);
    assert(str);
    assert(delim);

    *size = strcspn(str, delim);
    return str[*size];
}

void deleteMeaninglessSpaces(char* input) // todo: rewrite
{
    assert(input);
    size_t inputLength = strlen(input);
    size_t newStart = 0;
    int newEnd = int(inputLength) - 1;

    // Find the first non-space character.
    while (newStart < inputLength && input[newStart] == ' ')
    {
        newStart++;
    }

    // Find the last non-space character.
    while (newEnd > 0 && (input[newEnd] == ' ' || input[newEnd] == '\n'))
    {
        newEnd--;
    }
    int newLength = newEnd - (int)newStart;

    // Rewrite input without the spaces.
    for (int i = 0; i <= newLength; i++)
    {
        input[i] = input[i + newStart];
    }

    // End the string with '\0'.
    input[newLength + 1] = '\0';
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