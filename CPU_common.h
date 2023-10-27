#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdio.h>
#include <inttypes.h>

const int FLOATING_POINT_COEFFICIENT = 100;

struct Command
{
    const char* const name;
    const uint8_t code;
};

struct Register
{
    const char* name;
    const uint8_t id;
};



constexpr Command COMMANDS[] = 
{
    #define DEF_CMD(name, byte_code, ...) {#name, byte_code},
    #include "CPU_commands.h"
    #undef DEF_CMD
};

enum enumCommands // codestyle
{
    #define DEF_CMD(name, byte_code, ...) CMD_ ## name,
    #include "CPU_commands.h"
    #undef DEF_CMD
};

const size_t NUMBER_OF_CMD_BITS = 5;


const uint8_t COM_IMMEDIATE_BIT = 0b001'00000;
const uint8_t COM_REGISTER_BIT  = 0b010'00000;
const uint8_t COM_MEMORY_BIT    = 0b100'00000;
const uint8_t COM_COMMAND_BITS  = 0b000'11111;
 

const size_t REGISTER_LENGTH = 3;
const Register REGS[] =
{
    {"rax", 1},
    {"rbx", 2},
    {"rcx", 3},
    {"rdx", 4},
};


const size_t AMOUNT_OF_COMMANDS = sizeof(COMMANDS) / sizeof(Command);

const size_t AMOUNT_OF_REGISTERS = sizeof(REGS) / sizeof(Register);

#endif