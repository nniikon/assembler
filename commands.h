#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdio.h>
#include <inttypes.h>

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


/**
 * @brief An array of all commands, supported by SPU.
 * 
 *    reg imm  command id
 * V   V   V         V
 * 0   0   0 | 0  0  0  0  0
*/
constexpr Command COMMANDS[] = 
{
    {"push", 0b001'00001}, ///< push 5       001_00001     id = 01  0
    {"push", 0b010'00001}, ///< push rcx     010_00001     id = 01  1
    {"push", 0b011'00001}, ///< push rcx+5   011_00001     id = 01  2

    {"div",  0b000'00010}, ///< div          000_00010     id = 02  3
    {"sub",  0b000'00011}, ///< sub          000_00011     id = 03  4
    {"out",  0b000'00100}, ///< out          000_00100     id = 04  5

    {"HLT",  0b000'11111}, ///< HLT          000_11111     id = 31  6
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

#define MAX_COMMAND_LENGTH_DEF(func) func(5)

#define NUM(x) x
#define STR(x) #x

const size_t MAX_COMMAND_LENGTH = MAX_COMMAND_LENGTH_DEF(NUM);
const char* const MAX_COMMAND_LENGTH_FORMAT = MAX_COMMAND_LENGTH_DEF(STR);
const size_t AMOUNT_OF_REGISTERS = sizeof(REGS) / sizeof(Register);

#undef NUM
#undef STR
#undef MAX_COMMAND_LENGTH_DEF

#endif