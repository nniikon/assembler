#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdio.h>
#include <inttypes.h>

const int FLOATING_POINTER_COEFFICIENT = 100;

struct Command
{
    const char* const name;
    const u_int8_t code;
};

struct Register
{
    const char* name;
    const int id;
};



constexpr Command COMMANDS[] = 
{
    #define DEF_CMD(name, byte_code, has_reg, has_num, func_name) {#name, byte_code},
    #include "CPU_commands.h"
    #undef DEF_CMD
};

enum eCOMMANDS
{
    #define DEF_CMD(name, byte_code, has_reg, has_num, func_name) name ## has_reg ## has_num ## _ID,
    #include "CPU_commands.h"
    #undef DEF_CMD
};


const u_int8_t COM_REGISTER_BIT  = 0b010'00000;
const u_int8_t COM_IMMEDIATE_BIT = 0b001'00000;
const u_int8_t COM_COMMAND_BITS  = 0b000'11111;

const Register REGS[] =
{
    {"rax", 1},
    {"rbx", 2},
    {"rcx", 3},
    {"rdx", 4},
};


const size_t AMOUNT_OF_COMMANDS = sizeof(COMMANDS) / sizeof(Command);


const size_t AMOUNT_OF_REGISTERS = sizeof(REGS) / sizeof(Register);

#undef NUM
#undef STR
#undef MAX_COMMAND_LENGTH_DEF

#endif