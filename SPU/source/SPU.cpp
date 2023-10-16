#include "../include/SPU.h"


const int REG_POISON = INT32_MIN;


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
        for (int i_deb = 0; i_deb < spu->stack.size; i_deb++)\
        {\
            stackDump(&spu->stack, STACK_NO_ERROR);\
            DUMP_COMMAND("\tdata[%d] = <%d>", i_deb, spu->stack.data[i_deb]);\
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

static void push_num(SPU* spu)
{
    DUMP_PRINT("push_num( buffer <%p>, num <%d> )\n", spu->curCommand, *(spu->curCommand + 1));

    stackPush(&spu->stack, *(spu->curCommand + 1) * FLOATING_POINTER_COEFFICIENT);

    DUMP_SPU();

    spu->curCommand += 2;
}

static void push_reg(SPU* spu)
{
    DUMP_PRINT("push_num( buffer <%p>, reg <%d> )\n", spu->curCommand, *(spu->curCommand + 1));
    
    stackPush(&spu->stack, (spu->reg[getRegisterArrayNum(*(spu->curCommand + 1))]) * FLOATING_POINTER_COEFFICIENT);

    DUMP_SPU();
    
    spu->curCommand += 2;
}

static void push_reg_num(SPU* spu)
{
    DUMP_PRINT("push_num( buffer <%p>, reg <%d>, num <%d> )\n", spu->curCommand, *(spu->curCommand + 1), *(spu->curCommand + 2));
    
    stackPush(&spu->stack, (spu->reg[getRegisterArrayNum(*(spu->curCommand + 1))] + *(spu->curCommand + 2)) * FLOATING_POINTER_COEFFICIENT);

    DUMP_SPU();
    
    spu->curCommand += 3;
}

static void div(SPU* spu)
{
    DUMP_PRINT("div( buffer <%p> )\n", spu->curCommand);

    int a = 0;
    int b = 0;

    stackPop(&spu->stack, &a);
    stackPop(&spu->stack, &b);
    
    b *= FLOATING_POINTER_COEFFICIENT;
    
    stackPush(&spu->stack, b / a);

    DUMP_COMMAND("Division:\t <%d - %d = %d>", b, a, b / a);
    
    DUMP_SPU();

    spu->curCommand += 1;
}

static void sub(SPU* spu)
{
    DUMP_PRINT("sub( buffer <%p> )\n", spu->curCommand);
    int a = 0;
    int b = 0;
    
    stackPop(&spu->stack, &a);
    stackPop(&spu->stack, &b);

    stackPush(&spu->stack, b - a);

    DUMP_COMMAND("Subtraction:\t <%d - %d = %d>", b, a, b - a);
    
    DUMP_SPU();

    spu->curCommand += 1;
}

static void add(SPU* spu)
{
    DUMP_PRINT("add( buffer <%p> )\n", spu->curCommand);
    int a = 0;
    int b = 0;
    
    stackPop(&spu->stack, &a);
    stackPop(&spu->stack, &b);

    stackPush(&spu->stack, b + a);

    DUMP_COMMAND("Addition:\t <%d + %d = %d>", b, a, b + a);
    
    DUMP_SPU();
    
    spu->curCommand += 1;
}

static void out(SPU* spu)
{
    DUMP_PRINT("out( buffer <%p> )\n", spu->curCommand);

    int a = 0;

    stackPop(&spu->stack, &a);

    float fa = (float)a / FLOATING_POINTER_COEFFICIENT;

    fprintf(stdout, "\033[91m" "%g" "\033[0m\n", fa);

    DUMP_SPU();

    spu->curCommand += 1;
}

static void in(SPU* spu)
{
    DUMP_PRINT("in( buffer <%p> )\n", spu->curCommand);

    float a = 0;
    fscanf(stdin, "%g", &a);

    stackPush(&spu->stack, (int) (a * FLOATING_POINTER_COEFFICIENT));

    DUMP_COMMAND("In: pushed <%d> to stack", (int) (a * FLOATING_POINTER_COEFFICIENT));

    DUMP_SPU();

    spu->curCommand += 1;
}

static void mul(SPU* spu)
{
    DUMP_PRINT("mul( buffer <%p> )\n", spu->curCommand);

    int a = 0;
    int b = 0;
    
    stackPop(&spu->stack, &a);
    stackPop(&spu->stack, &b);

    stackPush(&spu->stack, a * b / FLOATING_POINTER_COEFFICIENT);

    DUMP_COMMAND("Multiplication:\t <%d * %d = %d>", a, b, a * b);

    DUMP_SPU();
    
    spu->curCommand += 1;
}

static void sqrt(SPU* spu)
{
    DUMP_PRINT("sqrt( buffer <%p> )\n", spu->curCommand);

    int a = 0;
    
    stackPop(&spu->stack, &a);

    stackPush(&spu->stack, (int)sqrt(a * FLOATING_POINTER_COEFFICIENT));

    DUMP_COMMAND("sqrt:\t <sqrt(%d) = %d>", a, (int)sqrt(a * FLOATING_POINTER_COEFFICIENT));

    DUMP_SPU();
    
    spu->curCommand += 1;
}

static void sin(SPU* spu)
{
    DUMP_PRINT("sin( buffer <%p> )\n", spu->curCommand);

    int a = 0;
    
    stackPop(&spu->stack, &a);

    stackPush(&spu->stack, (int) (FLOATING_POINTER_COEFFICIENT * sin(((float)a) / FLOATING_POINTER_COEFFICIENT)));

    DUMP_COMMAND("sin:\t <sin(%d) = %d>", a, (int)sin(a));

    DUMP_SPU();
    
    spu->curCommand += 1;
}

static void cos(SPU* spu)
{
    DUMP_PRINT("cos( buffer <%p> )\n", spu->curCommand);

    int a = 0;
    
    stackPop(&spu->stack, &a);

    stackPush(&spu->stack, (int) (FLOATING_POINTER_COEFFICIENT * cos(((float)a) / FLOATING_POINTER_COEFFICIENT)));

    DUMP_COMMAND("cos:\t <cos(%d) = %d>", a, (int)cos(a));

    DUMP_SPU();
    
    spu->curCommand += 1;
}

static void pop(SPU* spu)
{
    DUMP_PRINT("pop( buffer <%p> )\n", spu->curCommand);

    int a = 0;
    
    stackPop(&spu->stack, &a);

    spu->reg[getRegisterArrayNum(*(spu->curCommand + 1))] = a;

    DUMP_COMMAND("cos:\t <cos(%d) = %d>", a, (int)cos(a));

    DUMP_SPU();
    
    spu->curCommand += 2;
}

SPU_Error execProgram(SPU* spu)
{
    //  SIZE IS IN BYTES!!!!!!!!!!!!!!!!!!!
    DUMP_PRINT("execProgram( buffer <%p> )\n", spu->curCommand);

    while (true)
    {        
        DUMP_PRINT("currentBufferID = <%u>\n", *spu->curCommand);
        
        switch (*spu->curCommand)
        {
            case COMMANDS[PUSH_NUM_ID].code:        push_num    (spu);  break;
            case COMMANDS[PUSH_REG_ID].code:        push_reg    (spu);  break;
            case COMMANDS[PUSH_REG_NUM_ID].code:    push_reg_num(spu);  break;
            case COMMANDS[DIV_ID].code:             div         (spu);  break;
            case COMMANDS[SUB_ID].code:             sub         (spu);  break;
            case COMMANDS[OUT_ID].code:             out         (spu);  break;
            case COMMANDS[IN_ID].code:              in          (spu);  break;
            case COMMANDS[MUL_ID].code:             mul         (spu);  break;
            case COMMANDS[ADD_ID].code:             add         (spu);  break;
            case COMMANDS[SQRT_ID].code:            sqrt        (spu);  break; 
            case COMMANDS[SIN_ID].code:             sin         (spu);  break;
            case COMMANDS[COS_ID].code:             cos         (spu);  break;
            case COMMANDS[POP_ID].code:             pop         (spu);  break;
            case COMMANDS[HLT_ID].code: return SPU_NO_ERROR;            break;
            default:                    return SPU_INCORRECT_INPUT;     break;
        }
    }
    return SPU_NO_ERROR;
}

SPU_Error spuInit(SPU* spu, int* commandsArr)
{
    DUMP_PRINT("SPU initialization started:\n");

    // Error handling.
    if (spu == NULL)         return SPU_NULL_SPU;
    if (commandsArr == NULL) return SPU_NULL_ARRAY;

    // SPU init.
    spu->curCommand = commandsArr;
    
    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
        spu->reg[i] = REG_POISON;
    
    // Stack init.
    setStackLogFile("stackLog.htm"); // rename setStackLogFile

    StackError stackError = stackInit(&spu->stack);

    // Stack error handling.
    if (stackError != STACK_NO_ERROR)
    {
        DUMP_PRINT("Stack Error occured:\n");
        
        stackDump(&spu->stack, stackError);
        return SPU_STACK_ERROR;
    }

    DUMP_PRINT("SPU initialization ended successfully:\n");
    return SPU_NO_ERROR;
}

SPU_Error spuDtor(SPU* spu)
{
    DUMP_PRINT("SPU destruction started:\n");
    if (spu == NULL) return SPU_NULL_SPU;

    // Poison the SPU.
    spu->curCommand = NULL;

    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
        spu->reg[i] = REG_POISON;

    // Destruct the stack.
    StackError stackError = stackDtor(&spu->stack);
    if (stackError != STACK_NO_ERROR)
    {
        DUMP_PRINT("Stack Error occured:\n");

        return SPU_STACK_ERROR;
    }

    DUMP_PRINT("SPU destruction ended successfully:\n");
    return SPU_NO_ERROR;
}