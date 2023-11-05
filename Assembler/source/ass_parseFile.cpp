#include "../include/ass_parseFile.h"


void printTextToFile(Line* txt, FILE* file)
{
    for (int i = 0; txt[i].str; i++)
    {
        fwrite(txt[i].str, sizeof(char), txt[i].len, file);
        putc('\n', file);
    }
}


FileError FileToBuffer(char** buffer, const size_t size, const char* FILE_NAME)
{
    char* buf = (char*)calloc(size + 1, sizeof(char));
    if (buf == NULL)
    {
        //perror("Memory allocation error");
        return PARSE_MEM_ALLOCATION_ERROR;
    }

    FILE* file = fopen(FILE_NAME, "rb");
    if (file == NULL)
    {
        //perror("File opening error");
        return PARSE_FILE_OPEN_ERROR;
    }

    if (fread(buf, sizeof(char), size, file) != size)
    {
        //perror("Reading from file error.");
        return PARSE_FREAD_ERROR;
    }
    fclose(file);

    buf[size] = '\0';
    *buffer = buf;

    return PARSE_NO_ERROR;
}


FileError getFileSize(const char* fileName, size_t* size)
{
    struct stat bf = {};
    int error = stat(fileName, &bf);
    if (error == -1)
    {
        //perror("Stat error");
        return PARSE_STAT_ERROR;
    }

    *size = bf.st_size;
    return PARSE_NO_ERROR;
}


// This function accepts a string and a character as input.
// It scans through the string and removes consecutive occurrences of the specified character.
void deleteRepetitiveCharacters(char input[], const char chr)
{
    size_t currentEmptySpace = 0;
    // Iterate through the input string.
    for (size_t i = 0; input[i] != '\0'; ++i)
    {
        // If the current character is not the character to be deleted...
        if (input[i] != chr || input[i + 1] != chr)
        {
            // ... move the current character to the position of the current "empty" space and increment it.
            input[currentEmptySpace] = input[i];
            currentEmptySpace++;
        }
    }

    input[currentEmptySpace] = '\0';
}


void replaceCharacter(char input[], const char dst, const char src)
{
    for (int i = 0; input[i] != '\0'; i++)
    {
        if (input[i] == dst)
        {
            input[i] = src;
        }
    }
}


size_t nCharactersInString(const char input[], const char chr)
{
    size_t counter = 0;
    for (int i = 0; input[i] != '\0'; i++)
    {
        if (input[i] == chr)
        {
            counter++;
        }
    }
    return counter;
}


FileError parseBufferToLines(Line** dstLine, char* buffer, size_t* nLines, const char delimiter)
{
    // If there are 2 \n, there might be 3 Lines, hence + 1.
    *nLines = nCharactersInString(buffer, delimiter) + 1U;

    Line* Text = (Line*)calloc(*nLines + 1U, sizeof(Line));
    if (Text == NULL)
    {
        //perror("Memory allocation error");
        return PARSE_MEM_ALLOCATION_ERROR;
    }

    size_t Line = 0U;

    Text[Line].str = buffer;
    Line++;

    char* newLine = buffer;
    char* oldLine = buffer;
    while ((newLine = strchr(newLine, delimiter)) != NULL)
    {
        Text[Line - 1].len = ((size_t)newLine - (size_t)oldLine) / sizeof(char);
        Text[Line].str = ++newLine;
        oldLine = newLine;
        Line++;
    }
    Text[Line - 1].len = ((size_t)strchr(Text[Line - 1].str, '\0') - (size_t)Text[Line - 1].str) / sizeof(char);
    Text[*nLines].str = nullptr;

    *dstLine = Text;
    return PARSE_NO_ERROR;
}


FileError fileToNormilizedBuffer(const char* fileName, char** dstBuffer)
{
    size_t size = 0;
    FileError error = PARSE_NO_ERROR;

    error = getFileSize(fileName, &size);
    if (error != PARSE_NO_ERROR)
        return error;

    // MEM_WARNING: buffer was allocated.
    char* buffer = NULL;
    error = FileToBuffer(&buffer, size, fileName);
    if (error != PARSE_NO_ERROR)
    {
        free(buffer);
        return error;
    }

    // Replace different EOL symbols on '\n'.
    replaceCharacter          (buffer, '\r', '\n');
    deleteRepetitiveCharacters(buffer, '\n');

    *dstBuffer = buffer;

    return PARSE_NO_ERROR;
}


FileError textInit(const char* fileName, Text* txt)
{

    Line* ln = NULL;
    char* buffer = NULL; 
    FileError error = fileToNormilizedBuffer(fileName, &buffer);
    if (error != PARSE_NO_ERROR)
        return error;

    assert(buffer);
    size_t nLines = 0;

    // MEM_WARNING: ln was allocated.
    // Parse the buffer into the lines using \n as a delimiter.
    error = parseBufferToLines(&ln, buffer, &nLines, '\n');
    if (error != PARSE_NO_ERROR)
    {
        free(ln);
        free(buffer);
        return error;
    }

    assert(ln);
    assert(buffer);

    txt->line = ln;
    txt->buffer = buffer;
    txt->nLines = nLines;
    replaceCharacter(buffer, '\n', '\0');
    return PARSE_NO_ERROR;
}


void textDtor(Text* txt)
{
    assert(txt);
    assert(txt->line);
    assert(txt->buffer);

    free(txt->line);
    free(txt->buffer);
}
