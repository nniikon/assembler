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
    {"push", 0b001'00001}, ///< push 5       001_00001     id = 01   0
    {"push", 0b010'00001}, ///< push rcx     010_00001     id = 01   1
    {"push", 0b011'00001}, ///< push rcx+5   011_00001     id = 01   2

    {"div",  0b000'00010}, ///< div          000_00010     id = 02   3
    {"sub",  0b000'00011}, ///< sub          000_00011     id = 03   4
    {"out",  0b000'00100}, ///< out          000_00100     id = 04   5

    {"in",   0b000'00101}, ///< in           000_00101     id = 05   6
    {"mul",  0b000'00110}, ///< mul          000_00110     id = 06   7
    {"add",  0b000'00111}, ///< add          000_00111     id = 07   8
    {"sqrt", 0b000'01000}, ///< sqrt         000_01000     id = 08   9
    {"sin",  0b000'01001}, ///< sin          000_01001     id = 09  10
    {"cos",  0b000'01010}, ///< cos          000_01010     id = 10  11

    {"HLT",  0b000'11111}, ///< HLT          000_11111     id = 31  12
};

enum eCOMMANDS
{
    PUSH_NUM_ID     = 0,
    PUSH_REG_ID     = 1,
    PUSH_REG_NUM_ID = 2,
    DIV_ID          = 3,
    SUB_ID          = 4,
    OUT_ID          = 5,
    IN_ID           = 6,
    MUL_ID          = 7,
    ADD_ID          = 8,
    SQRT_ID         = 9,
    SIN_ID          = 10,
    COS_ID          = 11,
    HLT_ID          = 12,
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

#define NUM(x) x
#define STR(x) #x

const size_t AMOUNT_OF_REGISTERS = sizeof(REGS) / sizeof(Register);

#undef NUM
#undef STR
#undef MAX_COMMAND_LENGTH_DEF

#endif