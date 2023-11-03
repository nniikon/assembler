#include "../include/SPU.h"

#define DUMP_DEBUG
#include "../../lib/dump.h"

const int REG_POISON = INT32_MIN;


static int* getArgsAdress(SPU* spu)
{
    uint8_t cmd = *(uint8_t*)spu->curCommand;
    spu->curCommand += sizeof(uint8_t);

    int* res = NULL;
    int resSum = 0;

    if (cmd & CMD_REGISTER_BIT)
    {
        res   = &(spu->reg[*(uint8_t*)spu->curCommand]);
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


static int getArgsValue(SPU* spu)
{
    uint8_t cmd = ((uint8_t*)spu->curCommand)[0];

    int res = 0;

    // Since 'rax + 5' doesn't have its own variable, it needs to be handled individually.
    if ((cmd & CMD_REGISTER_BIT) && (cmd & CMD_IMMEDIATE_BIT) && !(cmd & CMD_MEMORY_BIT))
    {
        spu->curCommand += sizeof(uint8_t);
        res += (spu->reg[*(uint8_t*)spu->curCommand]);
        spu->curCommand += sizeof(uint8_t);
        res += *(int*)spu->curCommand;
        spu->curCommand += sizeof(int);
        return res;
    }


    return *getArgsAdress(spu);
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


SPU_Error spuInit(SPU* spu, const char* inputFileName)
{
    DUMP_PRINT("SPU initialization started:\n");

    assert(spu);
    assert(inputFileName);

    SPU_fileError fileErr = createBuffer(&(spu->commands), inputFileName);
    if (fileErr != SPU_FILE_NO_ERROR)
    {
        fprintf(stderr, "spuInit: file handling error\n");
        return SPU_FILE_ERROR;
    }

    // SPU init.
    spu->curCommand = spu->commands;

    // Poison the registers.
    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
        spu->reg[i] = REG_POISON;

    // Stack init.
    setStackLogFile("stackLog.htm");
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

    // Allocate memory for VRAM char buffer.                         VVVVV for '\n'
    char* tempVram = (char*) calloc(SPU_VRAM_HEIGHT * (SPU_VRAM_WIDTH + 1), sizeof(char));
    if (tempVram == NULL)
    {
        DUMP_PRINT("Error allocating memory for VRAM.\n");

        free(spu->ram);
        stackDtor(&spu->stack);
        return SPU_MEM_ALLOC_ERROR;
    }
    spu->vramBuffer = tempVram;

    DUMP_PRINT("SPU initialization ended successfully:\n");
    return SPU_NO_ERROR;
}


SPU_Error spuDtor(SPU* spu)
{
    DUMP_PRINT("SPU destruction started:\n");
    if (spu == NULL) return SPU_NULL_SPU;

    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
        spu->reg[i] = REG_POISON;

    // Destruct the stack.
    StackError stackError = stackDtor(&(spu->stack));
    if (stackError != STACK_NO_ERROR)
    {
        DUMP_PRINT("Stack Error occured:\n");
        free(spu->ram);
        free(spu->commands);
        free(spu->vramBuffer);
        return SPU_STACK_ERROR;
    }

    // Free the ram.
    free(spu->ram);
    free(spu->commands);
    free(spu->vramBuffer);

    // Poison the SPU.
    spu->curCommand = NULL;
    spu->commands   = NULL;


    DUMP_PRINT("SPU destruction ended successfully:\n");
    return SPU_NO_ERROR;
}