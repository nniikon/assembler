#define SGNTR_IMM 0b001'00000
#define SGNTR_REG 0b010'00000
#define SGNTR_RAM 0b100'00000

#define move_buffer_pos(shift) spu->curCommand += (shift);
#define set_buffer_pos(shift) spu->curCommand = spu->commands + shift;  
#define arg_value getArgsValue(spu);
#define arg_adress getArgsAdress(spu);
#define push(arg) stackPush(&spu->stack, arg);
#define pop(arg) stackPop(&spu->stack, &arg);
#define float_coef FLOATING_POINT_COEFFICIENT

// DEF_CMD (NAME, BYTE-CODE, SPU FUNCTION NAME)
DEF_CMD(PUSH, 0b000'00001 | SGNTR_IMM | SGNTR_RAM | SGNTR_REG,  
    {
        int arg = arg_value;

        DUMP_COMMAND("PUSH: <%g>\n", (float)arg / float_coef);

        push(arg);
    }
)

DEF_CMD(DIV, 0b000'00010, 
    {
        int value1 = 0;
        pop(value1);

        int value2 = 0;
        pop(value2);

        DUMP_COMMAND("DIV: <%g> / <%g> = <%g>\n", 
                    (float)value2 / float_coef, (float)value1 / float_coef, 
                    (float)(value2 * float_coef / value1) / float_coef);

        push(value2 * float_coef / value1);
        move_buffer_pos(sizeof(uint8_t));
    }
)

DEF_CMD(SUB, 0b000'00011,
    {
        int value1 = 0;
        pop(value1);

        int value2 = 0;
        pop(value2);

        DUMP_COMMAND("SUB: <%g> - <%g> = <%g>\n", 
                    (float)value2 / float_coef, (float)value1 / float_coef, 
                    (float)(value2 - value1) / float_coef);

        push(value2 - value1);
        move_buffer_pos(sizeof(uint8_t));
    }
)

DEF_CMD(ADD, 0b000'00111,     
    {                                 
        int value1 = 0;
        pop(value1);

        int value2 = 0;
        pop(value2);

        DUMP_COMMAND("ADD: <%g> + <%g> = <%g>\n", 
                    (float)value2 / float_coef, (float)value1 / float_coef, 
                    (float)(value2 + value1) / float_coef);

        push(value2 + value1);
        move_buffer_pos(sizeof(uint8_t));
    }
)

DEF_CMD(MUL, 0b000'00110,    
    {                                  
        int value1 = 0;
        pop(value1);

        int value2 = 0;
        pop(value2);

        DUMP_COMMAND("ADD: <%g> * <%g> = <%g>\n", 
                    (float)value2 / float_coef, (float)value1 / float_coef, 
                    (float)(value2 * value1 / float_coef) / float_coef);

        push(value2 * value1 / float_coef);
        move_buffer_pos(sizeof(uint8_t));
    }
)

DEF_CMD(OUT, 0b000'00100,
    {
        int value = 0;
        pop(value);

        fprintf(stdout, RED "%g" RESET "\n", (float)value / float_coef);
        
        move_buffer_pos(sizeof(uint8_t));
    }
)

DEF_CMD(IN, 0b000'00101,
    {
        float value = 0.0f;
        scanf("%g", &value);

        push((int)(value * float_coef));

        DUMP_COMMAND("IN: <%g>\n", (float)(int)(value * float_coef) / float_coef);

        move_buffer_pos(sizeof(uint8_t));
    }
)

DEF_CMD(SQRT, 0b000'01000,
    {
        int value = 0;
        pop(value);
        push((int)sqrt(value * float_coef));

        DUMP_COMMAND("SQRT: <%g> = <%g>\n", (float)value / float_coef, 
                                            (float)(int)sqrt(value * float_coef) / 100);

        move_buffer_pos(sizeof(uint8_t));
    }
)

DEF_CMD(SIN,  0b000'01001,
    {
        int value = 0;
        pop(value);
        push((int)(float_coef * sin((float)value / float_coef)));

        DUMP_COMMAND("SIN: <%g> = <%g>\n", (float)value / float_coef, 
                    (float)((int)(float_coef * sin((float)value / float_coef))) / float_coef);
        move_buffer_pos(sizeof(uint8_t));
    }
)

DEF_CMD(COS,  0b000'01010,
    {
        int value = 0;
        pop(value);
        push((int)(float_coef * cos((float)value / float_coef)));
        
        DUMP_COMMAND("COS: <%g> = <%g>\n", (float)value / float_coef, 
                    (float)((int)(float_coef * cos((float)value / float_coef))) / float_coef);
        move_buffer_pos(sizeof(uint8_t));
    }
)
DEF_CMD(POP,  0b000'01011 | SGNTR_REG | SGNTR_RAM | SGNTR_IMM,
    {
        int value = 0;
        pop(value);
        int* dst = arg_adress;
        *dst = value;
        DUMP_COMMAND("POP: <%g>\n", (float)value / float_coef);
    }
)
DEF_CMD(JUMP, 0b000'01100 | SGNTR_IMM,
    {
        int value = arg_value;
        set_buffer_pos(value);
    }
)

#define CONDITIONAL_JUMP(name, byte_code, condition)\
DEF_CMD(name, byte_code | SGNTR_IMM,\
    {\
        int value1 = 0;\
        pop(value1);\
\
        int value2 = 0;\
        pop(value2);\
\
        if (condition)\
        {\
            int value = arg_value;\
            set_buffer_pos(value);\
        }\
        else\
        {\
            move_buffer_pos(sizeof(uint8_t) + sizeof(int));\
        }\
    }\
)

CONDITIONAL_JUMP(ja,  0b000'01101, value2 >  value1)
CONDITIONAL_JUMP(jae, 0b000'01110, value2 >= value1)
CONDITIONAL_JUMP(jb,  0b000'01111, value2 <  value1)
CONDITIONAL_JUMP(jbe, 0b000'10000, value2 <= value1)
CONDITIONAL_JUMP(je,  0b000'10001, value2 == value1)
CONDITIONAL_JUMP(jne, 0b000'10010, value2 != value1)


DEF_CMD(HLT, 0b000'11111,
    {
        return SPU_NO_ERROR;
    }
)

#undef SGNTR_IMM
#undef SGNTR_REG
#undef SGNTR_RAM
#undef move_buffer_pos
#undef arg_value
#undef push
#undef pop
#undef float_coef


// PUSH | SGNT_REG -> push register
// PUSH | SGNT_IMM -> push immediate

// BYTE-CODE STRUCTURE
//ram reg imm    command id
// V   V   V   +-----V-----+  
// 0   0   0 | 0  0  0  0  0
