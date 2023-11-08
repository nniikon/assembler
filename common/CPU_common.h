#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdio.h>
#include <inttypes.h>
#include "CPU_constants.h"

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
    #include "CPU_commands_codegen.h"
    #undef DEF_CMD
};

enum enumCommands
{
    #define DEF_CMD(name, byte_code, ...) CMD_ ## name,
    #include "CPU_commands_codegen.h"
    #undef DEF_CMD
};

#define DEF_CMD(name, byte_code, ...) static_assert(byte_code <= CMD_COMMAND_BITS);
#undef DEF_CMD

const Register REGS[] =
{
    {"rax", 0},
    {"rbx", 1},
    {"rcx", 2},
    {"rdx", 3},
};

const size_t AMOUNT_OF_COMMANDS = sizeof(COMMANDS) / sizeof(Command);

const size_t AMOUNT_OF_REGISTERS = sizeof(REGS) / sizeof(Register);


#endif