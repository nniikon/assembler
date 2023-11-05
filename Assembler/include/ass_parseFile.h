#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>


enum FileError
{
    PARSE_NO_ERROR,
    PARSE_FILE_OPEN_ERROR,
    PARSE_MEM_ALLOCATION_ERROR,
    PARSE_STAT_ERROR,
    PARSE_INVALID_ARGS,
    PARSE_UNEXPECTED_ERROR,
    PARSE_FREAD_ERROR,
};


/// @brief A structure for storing a `char*` string along with it's length.
struct Line
{
    char* str;  /**< The `char*` string.       */
    size_t len; /**< The length of the string. */
};

struct Text
{
    Line* line;
    char* buffer;
    size_t nLines;
};


/**
 * @brief Prints a `Line*` Text into the file.
 *
 * @param txt   The struct Text array.
 * @param file  The opened for writing file.
 */
void printTextToFile(Line* txt, FILE* file);

/**
 * @brief Moves data from a file to the buffer.
 *
 * This function copies information from the provided file to the allocated char* array
 *
 * @param size[in]       A size of the file. 
 * @param FILE_NAME[in]  A name of the file.
 * @param buffer[in,out] A pointer to a buffer of the data from the file.
 * 
 * @return The `FileError` code.
 *
 * @note Don't forget to `free()` the given pointer.
 */
FileError FileToBuffer(char** buffer, const size_t size, const char* FILE_NAME);

/**
 * @brief Gets a file size by it's name.
 *
 * @param[in]  fileName The file name.
 * @param[out] size     Pointer to the size of the file to be set.
 * 
 * @return The `FileError` code.
 */
FileError getFileSize(const char* fileName, size_t* size);

/**
 * @brief Removes all consecutive occurrences of the given character in the string.
 *
 * @param[in,out] input The input string.
 * @param[in]     chr   The character to be removed.
 */
void deleteRepetitiveCharacters(char input[], const char chr);

/**
 * @brief Replaces every given symbol with another symbol.
 *
 * @param[in,out] input The input string.
 * @param[in]     dst   The symbol that will be replaced.
 * @param[in]     src   The symbol that will replace the given symbol.
 */
void replaceCharacter(char input[], const char dst, const char src);

/**
 * @brief Counts the amout of given characters in the string.
 *
 * @param[in] input The input string.
 * @param[in] chr   The character.
 * @return The amount of given characters in the string.
 */
size_t nCharactersInString(const char input[], const char chr);

/**
 * @brief Returns an array of pointer, each pointing to the start of a new line.
 *
 * @param[in]  buffer    The buffer that needs to be parsed.
 * @param[out] nLines    The number of lines in the buffer.
 * @param[in]  delimiter The symbol that indicates a new Line.
 * @param[out] dstLine   A pointer to the array on `Line`, ending with `NULL`.
 * 
 * @return The `FileError` code.
 *
 * @note Don't forget to `free()` the given pointer.
*/
FileError parseBufferToLines(Line** dstLine, char* buffer, size_t* nLines, const char delimiter);

/**
 * Takes data from a file and puts it into an allocated buffer.
 * This function replaced all EOL symbols with \\n.
 * The buffer is null-terminated.
 *
 * @param[in]  fileName  A name of the fuke
 * @param[out] dstBuffer A pointer to the buffer.
 * 
 * @return The `FileError` code.
 *
 * @note Don't forget to `free()` the given pointer.
*/
FileError fileToNormilizedBuffer(const char* fileName, char** dstBuffer);


FileError textInit(const char* fileName, Text* txt);


void textDtor(Text* txt);


#endif