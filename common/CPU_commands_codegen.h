// DEF_CMD (NAME, BYTE-CODE | SIGNATURES, SPU FUNCTION)
DEF_CMD(PUSH, 0b000'00001 | SGNTR_IMM | SGNTR_RAM | SGNTR_REG,
    {
        int arg = ARG_VALUE;

        PUSH(arg);
    }
)

DEF_CMD(DIV, 0b000'00010,
    {
        POP(value1);

        POP(value2);

        if (value1 == 0)
        {
            PRINT_STR("ERROR! Division by zero.\n");
            EXIT(DIVISION_BY_ZERO);
        }

        PUSH(TO_INT(TO_REAL(value2) * FLOAT_COEF / TO_REAL(value1)));
    }
)

DEF_CMD(SUB, 0b000'00011,
    {
        POP(value1);

        POP(value2);

        PUSH(value2 - value1);
    }
)

DEF_CMD(ADD, 0b000'00111,
    {
        POP(value1);

        POP(value2);

        PUSH(value2 + value1);
    }
)

DEF_CMD(MUL, 0b000'00110,
    {
        POP(value1);

        POP(value2);

        PUSH(TO_INT(TO_REAL(value2) * TO_REAL(value1) / FLOAT_COEF));
    }
)

DEF_CMD(OUT, 0b000'00100,
    {
        POP(value);

        PRINT_NUM(TO_REAL(value) / FLOAT_COEF);
    }
)

DEF_CMD(IN, 0b000'00101,
    {
        GET_REAL_NUM(value);
        PUSH(TO_INT(value * FLOAT_COEF));
    }
)

DEF_CMD(SQRT, 0b000'01000,
    {
        POP(value);
        PUSH(TO_INT(sqrt(value * FLOAT_COEF)));
    }
)

DEF_CMD(SIN,  0b000'01001,
    {
        POP(value);
        PUSH(TO_INT((FLOAT_COEF * sin(TO_REAL(value) / FLOAT_COEF))));
    }
)

DEF_CMD(COS,  0b000'01010,
    {
        POP(value);
        PUSH(TO_INT((FLOAT_COEF * cos(TO_REAL(value) / FLOAT_COEF))));
    }
)

DEF_CMD(POP,  0b000'01011 | SGNTR_REG | SGNTR_RAM | SGNTR_IMM,
    {
        POP(value);
        *ARG_ADRESS = value;
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
        int value = ARG_VALUE;\
        if (value2 condition value1)\
        {\
            SET_BUFFER_POS(value);\
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
    }
)

DEF_CMD(ret, 0b000'10100,
    {
        POP(value);
        SET_BUFFER_POS(value);
    }
)

DEF_CMD(meow, 0b000'10101 | SGNTR_IMM,
    {
        int value = ARG_VALUE / FLOAT_COEF;
        for (int i = 0; i < value; i++)
        {
            PRINT_STR("meow\n");
        }
    }
)

DEF_CMD(HLT, 0b000'11111,
    {
        RENDER;
        EXIT(NO_ERROR);
    }
)