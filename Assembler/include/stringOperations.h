#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H

#include <stdio.h>
#include <assert.h>
#include <cstring>


void deleteMeaninglessSpaces(char* input);

char getWordSize(size_t* size, const char* str, const char* delim);

#endif