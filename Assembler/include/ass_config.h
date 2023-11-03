#ifndef ASS_CONFIG_H
#define ASS_CONFIG_H


// The maximum amount of commands in the output file.
// The size of the output buffer depends on this value.
// Do not lower it.
const size_t MAX_NUMBER_LINE_CMD = 3;

// Size of the labels struct.
// Increase / decrease if needed. 
const size_t MAX_NUMBER_OF_LABELS = 128;

const size_t DEFAULT_SIZE_OF_ERROR_ARR = 16;

const char COMMENTS_CHR = ';';

#endif