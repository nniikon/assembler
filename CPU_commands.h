// To add a new command, you need to create a new DEF_CMD entry here.
// You should also create a corresponding function in the SPU.cpp file.

#define SGNTR_IMM 0b001'00000
#define SGNTR_REG 0b010'00000
#define SGNTR_RAM 0b100'00000

// DEF_CMD (NAME, BYTE-CODE, SPU FUNCTION NAME)
DEF_CMD(PUSH, 0b000'00001 | SGNTR_IMM | SGNTR_RAM | SGNTR_REG,  )
DEF_CMD(DIV,  0b000'00010,                                      )
DEF_CMD(SUB,  0b000'00011,                                      )
DEF_CMD(OUT,  0b000'00100,                                      )
DEF_CMD(IN,   0b000'00101,                                      )
DEF_CMD(MUL,  0b000'00110,                                      )
DEF_CMD(ADD,  0b000'00111,                                      )
DEF_CMD(SQRT, 0b000'01000,                                      )
DEF_CMD(SIN,  0b000'01001,                                      )
DEF_CMD(COS,  0b000'01010,                                      )
DEF_CMD(POP,  0b000'01011 | SGNTR_REG | SGNTR_RAM,              )
DEF_CMD(JUMP, 0b000'01100 | SGNTR_IMM,                          )
DEF_CMD(HLT,  0b000'11111,                                      )


// PUSH | SGNT_REG -> push register
// PUSH | SGNT_IMM -> push immediate

// BYTE-CODE STRUCTURE
//ram reg imm    command id
// V   V   V   +-----V-----+  
// 0   0   0 | 0  0  0  0  0
