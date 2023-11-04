// DEF_CMD (NAME, BYTE-CODE | SIGNATURES, SPU FUNCTION)
DEF_CMD(PUSH, 0b000'00001 | SGNTR_IMM | SGNTR_RAM | SGNTR_REG,
    {
        int arg = ARG_VALUE;

        PRINT_GREEN("PUSH: <%g>\n", (float)arg / FLOAT_COEF);

        PUSH(arg);
    }
)

DEF_CMD(DIV, 0b000'00010,
    {
        POP(value1);

        POP(value2);

        PRINT_GREEN("DIV: <%g> / <%g> = <%g>\n",
                    (float)value2 / FLOAT_COEF, (float)value1 / FLOAT_COEF,
                    (float)(value2 * FLOAT_COEF / value1) / FLOAT_COEF);

        PUSH((int)((float)value2 * FLOAT_COEF / (float)value1));
    }
)

DEF_CMD(SUB, 0b000'00011,
    {
        POP(value1);

        POP(value2);

        PRINT_GREEN("SUB: <%g> - <%g> = <%g>\n", 
                    (float)value2 / FLOAT_COEF, (float)value1 / FLOAT_COEF, 
                    (float)(value2 - value1) / FLOAT_COEF);

        PUSH(value2 - value1);
    }
)

DEF_CMD(ADD, 0b000'00111,
    {
        POP(value1);

        POP(value2);

        PRINT_GREEN("ADD: <%g> + <%g> = <%g>\n", 
                    (float)value2 / FLOAT_COEF, (float)value1 / FLOAT_COEF, 
                    (float)(value2 + value1) / FLOAT_COEF);

        PUSH(value2 + value1);
    }
)

DEF_CMD(MUL, 0b000'00110,
    {
        POP(value1);

        POP(value2);

        PRINT_GREEN("ADD: <%g> * <%g> = <%g>\n", 
                    (float)value2 / FLOAT_COEF, (float)value1 / FLOAT_COEF, 
                    (float)(value2 * value1 / FLOAT_COEF) / FLOAT_COEF);

        PUSH((int) ((float)value2 * (float)value1 / FLOAT_COEF));
    }
)

DEF_CMD(OUT, 0b000'00100,
    {
        POP(value);

        fprintf(stdout, RED "%g" RESET "\n", (float)value / FLOAT_COEF);

    }
)

DEF_CMD(IN, 0b000'00101,
    {
        float value = 0.0f;
        scanf("%g", &value);

        PUSH((int)(value * FLOAT_COEF));

        PRINT_GREEN("IN: <%g>\n", (float)(int)(value * FLOAT_COEF) / FLOAT_COEF);
    }
)

DEF_CMD(SQRT, 0b000'01000,
    {
        POP(value);
        PUSH((int)sqrt(value * FLOAT_COEF));

        PRINT_GREEN("SQRT: <%g> = <%g>\n", (float)value / FLOAT_COEF, 
                                            (float)(int)sqrt(value * FLOAT_COEF) / 100);
    }
)

DEF_CMD(SIN,  0b000'01001,
    {
        POP(value);
        PUSH((int)(FLOAT_COEF * sin((float)value / FLOAT_COEF)));

        PRINT_GREEN("SIN: <%g> = <%g>\n", (float)value / FLOAT_COEF, 
                    (float)((int)(FLOAT_COEF * sin((float)value / FLOAT_COEF))) / FLOAT_COEF);
    }
)

DEF_CMD(COS,  0b000'01010,
    {
        POP(value);
        PUSH((int)(FLOAT_COEF * cos((float)value / FLOAT_COEF)));

        PRINT_GREEN("COS: <%g> = <%g>\n", (float)value / FLOAT_COEF, 
                    (float)((int)(FLOAT_COEF * cos((float)value / FLOAT_COEF))) / FLOAT_COEF);
    }
)

DEF_CMD(POP,  0b000'01011 | SGNTR_REG | SGNTR_RAM | SGNTR_IMM,
    {
        POP(value);
        int* dst = ARG_ADRESS;
        *dst = value;
        PRINT_GREEN("POP: <%g>\n", (float)value / FLOAT_COEF);
    }
)

DEF_CMD(JUMP, 0b000'01100 | SGNTR_IMM,
    {
        SET_BUFFER_POS(ARG_VALUE);
    }
)

#define CONDITIONAL_JUMP(name, byte_code, condition)\
DEF_CMD(name, byte_code | SGNTR_IMM,\
    {\
        POP(value1);\
        POP(value2);\
\
        PRINT_GREEN("JUMP_CONDITION: %d V %d\n", value1, value2);\
        if (value2 condition value1)\
        {\
            int value = ARG_VALUE;\
            SET_BUFFER_POS(value);\
            PRINT_GREEN("Jump <%d>\n", value);\
        }\
        else\
        {\
            MOVE_BUFFER_POS(sizeof(int));\
        }\
    }\
)

CONDITIONAL_JUMP(ja,  0b000'01101, >  )
CONDITIONAL_JUMP(jae, 0b000'01110, >= )
CONDITIONAL_JUMP(jb,  0b000'01111, <  )
CONDITIONAL_JUMP(jbe, 0b000'10000, <= )
CONDITIONAL_JUMP(je,  0b000'10001, == )
CONDITIONAL_JUMP(jne, 0b000'10010, != )

DEF_CMD(CALL, 0b000'10011 | SGNTR_IMM,
    {
        int value = ARG_VALUE;
        PUSH(CUR_BUFFER_SHIFT);

        SET_BUFFER_POS(value);
        PRINT_GREEN("CALL: <%d>\n", value);
    }
)

DEF_CMD(ret, 0b000'10100,
    {
        POP(value);
        SET_BUFFER_POS(value);

        PRINT_GREEN("RET <%d>\n", value);
    }
)

DEF_CMD(meow, 0b000'10101 | SGNTR_IMM,
    {
        int value = ARG_VALUE / FLOAT_COEF;
        for (int i = 0; i < value; i++)
        {
            fprintf(stdout, "meow\n");
        }
    }
)

DEF_CMD(HLT, 0b000'11111,
    {
        RENDER;
        return SPU_NO_ERROR;
    }
)

#undef SGNTR_IMM
#undef SGNRT_RAM
#undef SGNRT_REG