#include "../include/stringOperations.h"

char getWordSize(size_t* size, const char* str, const char* delim)
{
    assert(size);
    assert(str);
    assert(delim);

    int delN = 0;
    for (*size = 0; str[*size] != '\0'; (*size)++)
    {
        for (delN = 0; delim[delN] != '\0'; delN++)
        {
            if (str[*size] == delim[delN])
            {
                return delim[delN];
            }
        }
    }
    return delim[delN];
}

void deleteMeaninglessSpaces(char* input)
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
    size_t newLength = newEnd - newStart;

    // Rewrite input without the spaces.
    for (size_t i = 0; i <= newLength; i++)
    {
        input[i] = input[i + newStart];
    }

    // End the string with '\0'.
    input[newLength + 1] = '\0';
}

