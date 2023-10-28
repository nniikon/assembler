#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H

#include <stdio.h>
#include <assert.h>
#include <cstring>


char getWordSize(size_t* size, const char* str, const char* delim);

void moveToNextWord(const char** input, size_t size, const char* delim);

bool chrAppearsMoreTimes(const char* str, const char chr, size_t amount);


#endif