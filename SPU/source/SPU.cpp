#include "../include/SPU.h"


Stack gSTACK  = {};
int   gREG[4] = {};

#define SPU_DEBUG

#ifdef SPU_DEBUG
    #define DUMP_PRINT(...)\
    do\
    {\
        fprintf(stderr, "file: %s line: %3d\t\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
    } while (0)

    #define DUMP_TEXT(txt)\
    do\
    {\
        fprintf(stderr, "file: %s line: %d\t\t", __FILE__, __LINE__);\
        fprintf(stderr, "length = %ld\n", txt->nLines);\
    } while (0)

    #define IF_SPU_DEBUG(...) __VA_ARGS__ 
#else
    #define DUMP_PRINT(...)   do {} while(0)
    #define DUMP_TEXT(txt)    do {} while(0) 
    #define IF_SPU_DEBUG(...) do {} while(0)
#endif



ParseError fileToIntBuffer(int** buffer, const size_t size, const char* FILE_NAME)
{
    int* buf = (int*)calloc(size, 1);
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

    fread(buf, size, 1, file); // TODO: error check

    fclose(file);

    *buffer = buf;
 
    return PARSE_NO_ERROR;
}


ParseError getFileSize(const char* fileName, size_t* size)
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

static void push_num(const int* buffer, size_t* shift)
{
    DUMP_PRINT("push_num( buffer <%p>, num <%d>)\n", buffer + *shift, *(buffer + (*shift + 1)));
    *shift += 2;
}

static void push_reg(const int* buffer, size_t* shift)
{
    DUMP_PRINT("push_num( buffer <%p>, reg <%d>)\n", buffer + *shift, *(buffer + (*shift + 1)));
    *shift += 2;
}

static void push_reg_num(const int* buffer, size_t* shift)
{
    DUMP_PRINT("push_num( buffer <%p>, reg <%d>, num <%d>)\n", buffer + *shift, *(buffer + (*shift + 1)), *(buffer + (*shift + 2)));
    *shift += 3;
}

static void div(const int* buffer, size_t* shift)
{
    DUMP_PRINT("div( buffer <%p>)\n", buffer + *shift);
    *shift += 1;
}

static void sub(const int* buffer, size_t* shift)
{
    DUMP_PRINT("sub( buffer <%p>)\n", buffer + *shift);
    *shift += 1;
}

static void out(const int* buffer, size_t* shift)
{
    DUMP_PRINT("out( buffer <%p>)\n", buffer + *shift);
}


SPU_Error execProgram(int* buffer, size_t size)
{
    //  SIZE IS IN BYTES!!!!!!!!!!!!!!!!!!!
    DUMP_PRINT("execProgram( buffer <%p>, size <%lu>)\n", buffer, size);

    size_t currentShift = 0;

    while (currentShift * sizeof(int) <= size)
    {
        u_int8_t currentBuffer = (u_int8_t) *(buffer + currentShift);
        DUMP_PRINT("currentBufferID = <%u>\n", currentBuffer);
        switch (currentBuffer)
        {
            case COMMANDS[0].code: push_num    (buffer, &currentShift); break;
            case COMMANDS[1].code: push_reg    (buffer, &currentShift); break;
            case COMMANDS[2].code: push_reg_num(buffer, &currentShift); break;
            case COMMANDS[3].code: div         (buffer, &currentShift); break;
            case COMMANDS[4].code: sub         (buffer, &currentShift); break;
            case COMMANDS[5].code: /*HLT*/      return SPU_NO_ERROR;    break;
            default: return SPU_INCORRECT_INPUT;                        break;
        }
    }
    return SPU_NO_ERROR;
}


SPU_Error spuInit()
{
    DUMP_PRINT("SPU initialization started:\n");
    StackError stackError = stackInit(&gSTACK);
    if (stackError != NO_ERROR)
    {
        DUMP_PRINT("Stack Error occured:\n");
        stackDump(&gSTACK, stackError);
        return SPU_STACK_ERROR;
    }
    DUMP_PRINT("SPU initialization ended successfully:\n");
    return SPU_NO_ERROR;
}

SPU_Error spuDtor()
{
    DUMP_PRINT("SPU destruction started:\n");
    StackError stackError = stackDtor(&gSTACK);
    if (stackError != NO_ERROR)
    {
        DUMP_PRINT("Stack Error occured:\n");
        return SPU_STACK_ERROR;
    }
    DUMP_PRINT("SPU destruction ended successfully:\n");
    return SPU_NO_ERROR;
}