#ifndef DUMP_H
#define DUMP_H

#include "colors.h"

#ifdef DUMP_DEBUG
    #define DUMP_PRINT(...)\
    do\
    {\
        fprintf(stderr, "file: %s line: %d\t\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
    } while (0)

    #define DUMP_PRINT_CYAN(...)\
    do\
    {\
        fprintf(stderr, CYAN);\
        DUMP_PRINT(__VA_ARGS__);\
        fprintf(stderr, RESET);\
    } while (0)

    #define PRINT_GREEN(...)\
    do\
    {\
        fprintf(stderr, GREEN __VA_ARGS__);\
        fprintf(stderr, RESET);\
    } while (0);

    #define IF_DUMP_DEBUG(...) __VA_ARGS__;
#else
    #define DUMP_PRINT(...) do {} while(0)
    #define DUMP_PRINT_CYAN(...) do {} while(0)
    #define PRINT_GREEN(...) do {} while(0)
    #define IF_DUMP_DEBUG(...) __VA_ARGS__;
#endif

#endif