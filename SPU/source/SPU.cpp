#include "../include/SPU.h"
//#define DUMP_DEBUG
#include "../../lib/dump.h"


const int REG_POISON = INT32_MIN;

//--+--------------------+
//  |        DSL         |
//  |       SYNTAX       |
//--+--------------------+

#define MOVE_BUFFER_POS(shift)\
    spu->curCommand += (shift)

#define SET_BUFFER_POS(shift)\
    spu->curCommand = spu->commands + shift

#define CUR_BUFFER_SHIFT\
    (int)((size_t)spu->curCommand - (size_t)spu->commands)

#define ARG_VALUE\
    getArgsValue(spu)

#define ARG_ADRESS\
    getArgsAdress(spu)

#define PUSH(arg)\
    {\
        StackError stkErr = stackPush(&spu->stack, arg);\
        if (stkErr != STACK_NO_ERROR)\
        {\
            DUMP_PRINT("ERROR! Stack push error.\n");\
            return SPU_STACK_ERROR;\
        }\
    }

#define POP(arg)\
    int arg = 0;\
    {\
        StackError stkErr = stackPop(&spu->stack, &arg);\
        if (stkErr != STACK_NO_ERROR)\
        {\
            DUMP_PRINT("ERROR! Stack pop error.\n");\
            return SPU_STACK_ERROR;\
        }\
    }

#define FLOAT_COEF\
    FLOATING_POINT_COEFFICIENT

#define RENDER\
    renderRam(spu->ram, spu->vramBuffer, SPU_RAM_CAPACITY)

#define TO_REAL(arg) (float)(arg)

#define TO_INT(arg) (int)(arg)

#define PRINT_NUM(...) fprintf(stdout, GREEN "%g\n" RESET, __VA_ARGS__)

#define PRINT_STR(...) fprintf(stdout, CYAN __VA_ARGS__ RESET)

#define GET_REAL_NUM(var)\
    float var = 0.0f;\
    scanf("%g", &var)

#define EXIT(err) return SPU_ ## err;

//------------------------


static int* getArgsAdress(SPU* spu)
{
    /*  curCommand
           V
       |AB|00|11 11 11 11|
        |  |  |
        |  |  +-->imm
        |  +----->reg
        +-------->opcode 
    */
    int* res = NULL;
    int resSum = 0;

    if (spu->opcode & CMD_REGISTER_BIT)
    {
        int regID = ((uint8_t*)spu->curCommand)[0];
        res = &(spu->reg[regID]);

        int regValue = *res;
        resSum += regValue;

        size_t shift = sizeof(uint8_t);
        spu->curCommand += shift;

        if (spu->isDump)
        {
            spu->dump.cmdInfo.hasReg = true;
            spu->dump.cmdInfo.regID = regID;
            spu->dump.cmdInfo.regValue = regValue;
            spu->dump.shift += shift;
        }
    }
    if (spu->opcode & CMD_IMMEDIATE_BIT)
    {
        res = (int*)spu->curCommand;

        int imm = *res;
        resSum += imm;

        size_t shift = sizeof(int);
        spu->curCommand += shift;

        if (spu->isDump)
        {
            spu->dump.cmdInfo.hasImm = true;
            spu->dump.cmdInfo.imm = imm;
            spu->dump.shift += shift;
        }
    }
    if (spu->opcode & CMD_MEMORY_BIT)
    {
        int mem = resSum / FLOATING_POINT_COEFFICIENT;
        res = &(spu->ram[mem]);

        if (spu->isDump)
        {
            spu->dump.cmdInfo.hasMem = true;
            spu->dump.cmdInfo.mem = mem;
        }
    }

    return res;
}


static int getArgsValue(SPU* spu)
{
    int res = 0;

    // Since 'rax + 5' doesn't have its own variable, it needs to be handled individually.
    if ((spu->opcode & CMD_REGISTER_BIT) && (spu->opcode & CMD_IMMEDIATE_BIT) && !(spu->opcode & CMD_MEMORY_BIT))
    {
        int regID = ((uint8_t*)spu->curCommand)[0];
        int regValue = spu->reg[regID];
        res += regValue;
        spu->curCommand += sizeof(uint8_t);

        int imm = ((int*)spu->curCommand)[0];
        res += imm;
        spu->curCommand += sizeof(int);

        if (spu->isDump)
        {
            spu->dump.cmdInfo.hasImm = true;
            spu->dump.cmdInfo.hasReg = true;
            spu->dump.shift += sizeof(uint8_t) + sizeof(int);
            spu->dump.cmdInfo.imm = imm;
            spu->dump.cmdInfo.regID = regID;
            spu->dump.cmdInfo.regValue = regValue;
        }
        return res;
    }

    if (spu->isDump)
        spu->dump.isResValue = true;

    return *getArgsAdress(spu);
}


static void setDump(SPU* spu, const char* name)
{
    spu->dump.cmdInfo.hasImm = 0;
    spu->dump.cmdInfo.hasReg = 0;
    spu->dump.cmdInfo.hasMem = 0;
    spu->dump.isResValue = false;
    spu->dump.cmdName = name;
    spu->dump.shift = 1ul; // since opcode is guaranteed.
    spu->dump.adress = (size_t)spu->curCommand - (size_t)spu->commands;
    spu->dump.startAdressPtr = spu->curCommand;
}


SPU_Error execProgram(SPU* spu)
{
    DUMP_PRINT("execProgram( buffer <%p> )\n", spu->curCommand);

    while (true)
    {
        DUMP_PRINT("currentBufferID = <%u>\n", *spu->curCommand);
        DUMP_PRINT("currectAdress   = <%zu>\n", (size_t)spu->curCommand - (size_t)spu->commands);

        spu->opcode = ((uint8_t*)spu->curCommand)[0];
        switch (spu->opcode & CMD_COMMAND_BITS)
        {
            #define DEF_CMD(name, byte_code, ...)                                       \
                case COMMANDS[CMD_ ## name].code & CMD_COMMAND_BITS:                    \
                    if (spu->isDump)                                                    \
                        setDump(spu, #name);                                            \
                                                                                        \
                    spu->curCommand += sizeof(uint8_t);                                 \
                    __VA_ARGS__;                                                        \
                                                                                        \
                    if (spu->isDump)                                                    \
                        dumpSpu(&(spu->dump), spu->reg, spu->ram, &spu->stack);         \
                break;

            #include "../../common/CPU_commands_codegen.h"

            #undef DEF_CMD

            default:
                return SPU_INCORRECT_INPUT;
                break;
        }
        // renderRam(spu->ram, SPU_RAM_CAPACITY);
    }
    return SPU_NO_ERROR;
}


SPU_Error spuInit(SPU* spu, SPU_InitData* data)
{
    DUMP_PRINT("SPU initialization started:\n");

    assert(spu);
    assert(data);
    assert(data->inputFileName);

    if (data->inputFileName == NULL)
        return SPU_FILE_ERROR;

    SPU_fileError fileErr = createBinaryBuffer(&(spu->commands), data->inputFileName);
    if (fileErr != SPU_FILE_NO_ERROR)
    {
        fprintf(stderr, "spuInit: file handling error\n");
        return SPU_FILE_ERROR;
    }

    // Open dump file.
    if (data->extDumpFileName != NULL || data->dumpFileName != NULL)
    {
        spu->isDump = true;
        const char* dumpFileName;

        if (data->extDumpFileName != NULL)
        {
            spu->dump.isExtended = true;
            dumpFileName = data->extDumpFileName;
        }
        else
        {
            dumpFileName = data->dumpFileName;
        }

        spu->dump.file = fopen(dumpFileName, "w");
        if (spu->dump.file == NULL)
        {
            fprintf(stderr, "spuInit: dump file handling error\n");
            return SPU_FILE_ERROR;
        }
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

#undef SGNTR_IMM
#undef SGNTR_REG
#undef SGNTR_RAM
#undef MOVE_BUFFER_POS
#undef ARG_VALUE
#undef ARG_ADRESS
#undef PUSH
#undef POP
#undef FLOAT_COEF
#undef RENDER