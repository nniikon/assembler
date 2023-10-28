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
        fprintf(stderr, GREEN __VA_ARGS__);\
        fprintf(stderr, RESET);\
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


ParseError fileToIntBuffer(uint8_t** buffer, const size_t size, const char* FILE_NAME)
{
    uint8_t* tempBuf = (uint8_t*)calloc(size, 1);
    if (tempBuf == NULL)
    {
        return PARSE_MEM_ALLOCATION_ERROR;
    }

    FILE* file = fopen(FILE_NAME, "rb");
    if (file == NULL)
    {
        return PARSE_FILE_OPEN_ERROR;
    }

    size_t sizeRef = fread(tempBuf, 1, size, file);
    if (sizeRef != size)
    {
        return PARSE_FREAD_ERROR;
    }

    fclose(file);

    *buffer = tempBuf;
 
    return PARSE_NO_ERROR;
}


ParseError getFileSize(const char* fileName, size_t* size)
{
    struct stat bf = {};
    int error = stat(fileName, &bf);
    if (error == -1)
    {
        return PARSE_STAT_ERROR;
    }

    *size = bf.st_size;
    return PARSE_NO_ERROR;
}


static int getArgsValue(SPU* spu)
{
    uint8_t cmd = ((uint8_t*)spu->curCommand)[0];
    spu->curCommand += sizeof(uint8_t);

    int res = 0;

    if (cmd & CMD_REGISTER_BIT)
    {
        res += spu->reg[*(uint8_t*)spu->curCommand];
        spu->curCommand += sizeof(uint8_t);
    }
    if (cmd & CMD_IMMEDIATE_BIT)
    {
        res += *(int*)spu->curCommand;
        spu->curCommand += sizeof(int);
    }
    if (cmd & CMD_MEMORY_BIT)
    {
        res = spu->ram[res / FLOATING_POINT_COEFFICIENT];
    }

    return res;
}


static int* getArgsAdress(SPU* spu)
{
    uint8_t cmd = *(uint8_t*)spu->curCommand;
    spu->curCommand += sizeof(uint8_t);

    int* res = NULL;
    int resSum = 0;

    if (cmd & CMD_REGISTER_BIT)
    {
        res = &(spu->reg[*(uint8_t*)spu->curCommand]);
        resSum += spu->reg[*(uint8_t*)spu->curCommand];
        spu->curCommand += sizeof(uint8_t);
    }
    if (cmd & CMD_IMMEDIATE_BIT)
    {
        res = (int*)spu->curCommand;
        resSum += *(int*)spu->curCommand;
        spu->curCommand += sizeof(int);
    }
    if (cmd & CMD_MEMORY_BIT)
    {
        res = &(spu->ram[resSum / FLOATING_POINT_COEFFICIENT]);
    }

    return res;
}


SPU_Error execProgram(SPU* spu)
{
    DUMP_PRINT("execProgram( buffer <%p> )\n", spu->curCommand);

    while (true)
    {
        DUMP_PRINT("currentBufferID = <%u>\n", *spu->curCommand);
        DUMP_PRINT("currectAdress   = <%zu>\n", (size_t)spu->curCommand - (size_t)spu->commands);

        switch (*(uint8_t*)spu->curCommand & CMD_COMMAND_BITS)
        {
            #define DEF_CMD(name, byte_code, ...)\
                case COMMANDS[CMD_ ## name].code & CMD_COMMAND_BITS: __VA_ARGS__; break;

            #include "../../CPU_commands.h"

            #undef DEF_CMD

            default:
                return SPU_INCORRECT_INPUT;
                break;
        }
        // renderRam_console(spu->ram, SPU_RAM_CAPACITY);
    }
    return SPU_NO_ERROR;
}


SPU_Error spuInit(SPU* spu, uint8_t* commandsArr)
{
    DUMP_PRINT("SPU initialization started:\n");

    // Error handling.
    if (spu == NULL)         return SPU_NULL_SPU;
    if (commandsArr == NULL) return SPU_NULL_ARRAY;

    // SPU init.
    spu->curCommand = commandsArr;
    spu->commands   = commandsArr;
    
    // Poison the registers.
    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
        spu->reg[i] = REG_POISON;
    
    // Stack init.
    setStackLogFile("stackLog.htm"); // TODO: FIX CRINGE

    // Stack init.
    StackError stackError = stackInit(&spu->stack);
    // Stack error handling.
    if (stackError != STACK_NO_ERROR)
    {
        DUMP_PRINT("Stack Error occured:\n");
        
        stackDump(&spu->stack, stackError);
        return SPU_STACK_ERROR;
    }

    // Allocate memory for RAM.
    int* tempRam = (int*) calloc(SPU_RAM_CAPACITY, sizeof(int));
    if (tempRam == NULL)
    {
        DUMP_PRINT("Error allocating memory for RAM.\n");
        
        stackDtor(&spu->stack);
        return SPU_MEM_ALLOC_ERROR;
    }
    spu->ram = tempRam;



    DUMP_PRINT("SPU initialization ended successfully:\n");
    return SPU_NO_ERROR;
}


SPU_Error spuDtor(SPU* spu)
{
    DUMP_PRINT("SPU destruction started:\n");
    if (spu == NULL) return SPU_NULL_SPU;

    // Poison the SPU.
    spu->curCommand = NULL;
    spu->commands   = NULL;

    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
        spu->reg[i] = REG_POISON;

    // Destruct the stack.
    StackError stackError = stackDtor(&(spu->stack));
    if (stackError != STACK_NO_ERROR)
    {
        DUMP_PRINT("Stack Error occured:\n");

        return SPU_STACK_ERROR;
    }

    // Free the ram.
    free(spu->ram);

    DUMP_PRINT("SPU destruction ended successfully:\n");
    return SPU_NO_ERROR;
}