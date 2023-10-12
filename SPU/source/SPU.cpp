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

    #define DUMP_COMMAND(...)\
    do\
    {\
        fprintf(stderr, "\033[92m\t" __VA_ARGS__);\
        fprintf(stderr, "\033[0m\n");\
    } while (0);
    
    #define DUMP_SPU()\
    do\
    {\
        for (int i_deb = 0; i_deb < gSTACK.size; i_deb++)\
        {\
            stackDump(&gSTACK, NO_ERROR);\
            DUMP_COMMAND("\tdata[%d] = <%d>", i_deb, gSTACK.data[i_deb]);\
        }\
    } while (0);
    

    #define IF_SPU_DEBUG(...) __VA_ARGS__ 
#else
    #define DUMP_PRINT(...)   do {} while(0)
    #define DUMP_TEXT(txt)    do {} while(0) 
    #define IF_SPU_DEBUG(...) do {} while(0)
    #define DUMP_COMMAND(...) do {} while(0)
    #define DUMP_SPU          do {} while(0)
#endif


static int getRegisterArrayNum(int reg)
{
    // Dumb function
    // TODO: fix / add enum / structure
    return reg - 1;
}

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

    stackPush(&gSTACK, *(buffer + (*shift + 1)) * FLOATING_POINTER_COEFFICIENT);

    DUMP_SPU();

    *shift += 2;
}

static void push_reg(const int* buffer, size_t* shift)
{
    DUMP_PRINT("push_num( buffer <%p>, reg <%d>)\n", buffer + *shift, *(buffer + (*shift + 1)));
    
    stackPush(&gSTACK, (gREG[getRegisterArrayNum(*(buffer + (*shift + 1)))]) * FLOATING_POINTER_COEFFICIENT);

    DUMP_SPU();
    
    *shift += 2;
}

static void push_reg_num(const int* buffer, size_t* shift)
{
    DUMP_PRINT("push_num( buffer <%p>, reg <%d>, num <%d>)\n", buffer + *shift, *(buffer + (*shift + 1)), *(buffer + (*shift + 2)));
    
    stackPush(&gSTACK, (gREG[getRegisterArrayNum(*(buffer + (*shift + 1)))] + *(buffer + (*shift + 2))) * FLOATING_POINTER_COEFFICIENT);

    DUMP_SPU();
    
    *shift += 3;
}

static void div(const int* buffer, size_t* shift)
{
    DUMP_PRINT("div( buffer <%p>)\n", buffer + *shift);

    int a = 0;
    int b = 0;

    stackPop(&gSTACK, &a);
    stackPop(&gSTACK, &b);
    
    b *= FLOATING_POINTER_COEFFICIENT;
    
    stackPush(&gSTACK, b / a);

    DUMP_COMMAND("Division:\t <%d - %d = %d>", b, a, b / a);
    
    DUMP_SPU();

    *shift += 1;
}

static void sub(const int* buffer, size_t* shift)
{
    DUMP_PRINT("sub( buffer <%p>)\n", buffer + *shift);
    int a = 0;
    int b = 0;
    
    stackPop(&gSTACK, &a);
    stackPop(&gSTACK, &b);

    stackPush(&gSTACK, b - a);

    DUMP_COMMAND("Subtraction:\t <%d - %d = %d>", b, a, b - a);
    
    DUMP_SPU();

    *shift += 1;
}

static void add(const int* buffer, size_t* shift)
{
    DUMP_PRINT("add( buffer <%p>)\n", buffer + *shift);
    int a = 0;
    int b = 0;
    
    stackPop(&gSTACK, &a);
    stackPop(&gSTACK, &b);

    stackPush(&gSTACK, b + a);

    DUMP_COMMAND("Addition:\t <%d + %d = %d>", b, a, b + a);
    
    DUMP_SPU();
    
    *shift += 1;
}

static void out(const int* buffer, size_t* shift)
{
    DUMP_PRINT("out( buffer <%p>)\n", buffer + *shift);

    int a = 0;
    stackPop(&gSTACK, &a);

    fprintf(stdout, "\033[91m" "%d,%d" "\033[0m\n", a / FLOATING_POINTER_COEFFICIENT, a % FLOATING_POINTER_COEFFICIENT);

    DUMP_SPU();

    *shift += 1;
}

static void in(const int* buffer, size_t* shift)
{
    DUMP_PRINT("in( buffer <%p>)\n", buffer + *shift);

    float a = 0;
    fscanf(stdin, "%d", &a);

    stackPush(&gSTACK, a * FLOATING_POINTER_COEFFICIENT);

    DUMP_COMMAND("In: pushed <%d> to stack", a);

    DUMP_SPU();

    *shift += 1;
}

static void mul(const int* buffer, size_t* shift)
{
    DUMP_PRINT("mul( buffer <%p>)\n", buffer + *shift);

    int a = 0;
    int b = 0;
    
    stackPop(&gSTACK, &a);
    stackPop(&gSTACK, &b);

    stackPush(&gSTACK, a * b / FLOATING_POINTER_COEFFICIENT);

    DUMP_COMMAND("Multiplication:\t <%d * %d = %d>", a, b, a * b);

    DUMP_SPU();
    
    *shift += 1;
}

static void sqrt(const int* buffer, size_t* shift)
{
    DUMP_PRINT("sqrt( buffer <%p>)\n", buffer + *shift);

    int a = 0;
    
    stackPop(&gSTACK, &a);

    stackPush(&gSTACK, (int)sqrt(a * FLOATING_POINTER_COEFFICIENT));

    DUMP_COMMAND("sqrt:\t <sqrt(%d) = %d>", a, (int)sqrt(a * FLOATING_POINTER_COEFFICIENT));

    DUMP_SPU();
    
    *shift += 1;
}

static void sin(const int* buffer, size_t* shift)
{
    DUMP_PRINT("sin( buffer <%p>)\n", buffer + *shift);

    int a = 0;
    
    stackPop(&gSTACK, &a);

    a /= FLOATING_POINTER_COEFFICIENT;

    stackPush(&gSTACK, (int) (sin(a) * FLOATING_POINTER_COEFFICIENT));

    DUMP_COMMAND("sin:\t <sin(%d) = %d>", a, (int)sin(a));

    DUMP_SPU();
    
    *shift += 1;
}

static void cos(const int* buffer, size_t* shift)
{
    DUMP_PRINT("cos( buffer <%p>)\n", buffer + *shift);

    int a = 0;
    
    stackPop(&gSTACK, &a);

    a /= FLOATING_POINTER_COEFFICIENT;

    stackPush(&gSTACK, (int) (cos(a) * FLOATING_POINTER_COEFFICIENT));

    DUMP_COMMAND("cos:\t <cos(%d) = %d>", a, (int)cos(a));

    DUMP_SPU();
    
    *shift += 1;
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
            case COMMANDS[PUSH_NUM_ID].code:        push_num    (buffer, &currentShift); break;
            case COMMANDS[PUSH_REG_ID].code:        push_reg    (buffer, &currentShift); break;
            case COMMANDS[PUSH_REG_NUM_ID].code:    push_reg_num(buffer, &currentShift); break;
            case COMMANDS[DIV_ID].code:             div         (buffer, &currentShift); break;
            case COMMANDS[SUB_ID].code:             sub         (buffer, &currentShift); break;
            case COMMANDS[OUT_ID].code:             out         (buffer, &currentShift); break;
            case COMMANDS[IN_ID].code:              in          (buffer, &currentShift); break;
            case COMMANDS[MUL_ID].code:             mul         (buffer, &currentShift); break;
            case COMMANDS[ADD_ID].code:             add         (buffer, &currentShift); break;
            case COMMANDS[SQRT_ID].code:            sqrt        (buffer, &currentShift); break; 
            case COMMANDS[SIN_ID].code:             sin        (buffer, &currentShift);  break;
            case COMMANDS[COS_ID].code:             cos        (buffer, &currentShift);  break;
            case COMMANDS[HLT_ID].code: return SPU_NO_ERROR;                             break;
            default:                    return SPU_INCORRECT_INPUT;                      break;
        }
    }
    return SPU_NO_ERROR;
}

SPU_Error spuInit()
{
    DUMP_PRINT("SPU initialization started:\n");
    setStackLogFile("stackLog.htm"); // rename setStackLogFile
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