#include "../include/SPU_dump.h"

#define print(...) fprintf(dump->file, __VA_ARGS__)
#define put(...) putc(__VA_ARGS__, dump->file)


static void dumpRegs(SpuDumpInfo* dump, int* regs)
{
    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
    {
        print("%d: %s: <%d>\n", REGS[i].id, REGS[i].name, regs[i]);
    }
}


static void dumpRam(SpuDumpInfo* dump, int* ram)
{
    // TODO: optimize -> put to buffer first
    const int BASE = 32;
    const int SPACE = 3;

    print("%*c", SPACE + 1, ' ');
    for (int i = 0; i < BASE; i++)
    {
        print("%*x", SPACE + 1, i);
    }
    put('\n');
    for (int i = 0; i < (int) SPU_RAM_CAPACITY; i++)
    {
        if (i % BASE == 0)
            print("%*x |", SPACE + 2, i);
        else
            print("%*c", SPACE, ' ');


        if (ram[i] == 0)
            put('.');
        else
            put('*');

        if (i % BASE == BASE - 1)
            put('\n');
    }
    put('\n');

}


static void dumpStack(SpuDumpInfo* dump, Stack* stack)
{
    const int N_ELEMS = 10;

    int i = N_ELEMS;
    int size = stack->size;

    while (--size >= 0 && i-- >= 0)
    {
        print("stack[%2d] | %2d\n", size, stack->data[size]);
    }
}


void dumpSpu(SpuDumpInfo* dump, int* regs, int* ram, Stack* stk)
{
    const int NUM_LENGTH = 2;
    const int BYTE_BUFFER_SIZE = (sizeof(int) + 2 * sizeof(uint8_t)) * (NUM_LENGTH + 1) + 1;


    assert(dump);
    assert(dump->file);
    assert(dump->cmdName);

    print("%5zu | ", dump->adress);

    char buffer[BYTE_BUFFER_SIZE] = {};
    char *bufIndex = buffer;
    for (;dump->shift != 0; dump->shift--)
    {
        sprintf(bufIndex, "%02X ", dump->startAdressPtr[0]);
        bufIndex += NUM_LENGTH + 1;
        dump->startAdressPtr++;
    }

    print("%*s| ", BYTE_BUFFER_SIZE - 1, buffer);

    print("%5s ", dump->cmdName);

    if (dump->cmdInfo.hasMem)
        print("[ ");
    else
        print("%*c", NUM_LENGTH, ' ');

    if (dump->cmdInfo.hasReg)
    {
        print("%s ", REGS[dump->cmdInfo.regID].name);
        if (dump->isResValue)
            print("= <%d>", dump->cmdInfo.regValue);
        if (dump->cmdInfo.hasImm && dump->cmdInfo.hasReg)
            print("+ ");
    }

    if (dump->cmdInfo.hasImm)
        print("%d ", dump->cmdInfo.imm);

    if (dump->cmdInfo.hasMem)
    {
        print("]");
        if (dump->isResValue)
            print(" = %d", ram[dump->cmdInfo.imm / FLOATING_POINT_COEFFICIENT]);
    }
    put('\n');

    if (dump->isExtended)
    {
        dumpRegs(dump, regs);
        dumpRam(dump, ram);
        dumpStack(dump, stk);
        put('\n');
    }
}