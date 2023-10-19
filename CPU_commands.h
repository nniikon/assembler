// To add a new command, you need to create a new DEF_CMD entry here.
// You should also create a corresponding function in the SPU.cpp file.

// DEF_CMD (NAME, BYTE-CODE, HAS REG, HAS NUM, SPU FUNCTION NAME)
DEF_CMD(PUSH, 0b001'00001, 0, 1, push_num    )
DEF_CMD(PUSH, 0b010'00001, 1, 0, push_reg    )
DEF_CMD(PUSH, 0b011'00001, 1, 1, push_reg_num)
DEF_CMD(DIV,  0b000'00010, 0, 0, div         )
DEF_CMD(SUB,  0b000'00011, 0, 0, sub         )
DEF_CMD(OUT,  0b000'00100, 0, 0, out         )
DEF_CMD(IN,   0b000'00101, 0, 0, in          )
DEF_CMD(MUL,  0b000'00110, 0, 0, mul         )
DEF_CMD(ADD,  0b000'00111, 0, 0, add         )
DEF_CMD(SQRT, 0b000'01000, 0, 0, sqrt        )
DEF_CMD(SIN,  0b000'01001, 0, 0, sin         )
DEF_CMD(COS,  0b000'01010, 0, 0, cos         )
DEF_CMD(POP,  0b010'01011, 1, 0, pop         )
DEF_CMD(HLT,  0b000'11111, 0, 0, HLT         )


// BYTE-CODE STRUCTURE
//    reg imm    command id
// V   V   V   +-----V-----+  
// 0   0   0 | 0  0  0  0  0
