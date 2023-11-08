#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

char getWordSize1(size_t* size, const char* str, const char* delim);

void moveToNextWord1(const char** input, size_t size, const char* delim);

size_t nChrInLine(const char* str, const char chr);

const char* skipWhiteSpaces(const char* input);

size_t getWordSize(const char* input, const char* delims);

const char* skipDelims(const char* input, const char* delims);


#endif