#include "../include/assembler.h"

const int ERROR_ID = -336;

#define ASSEMBLER_DEBUG   

#ifdef ASSEMBLER_DEBUG
    #define DUMP_PRINT(...)\
    do\
    {\
        fprintf(stderr, "file: %s line: %d\t\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
    } while (0)

    #define DUMP_TEXT(txt)\
    do\
    {\
        fprintf(stderr, "file: %s line: %d\t\t", __FILE__, __LINE__);\
        fprintf(stderr, "length = %ld\n", txt->nLines);\
    } while (0)

    #define IF_ASSEMBLER_DEBUG(...) __VA_ARGS__ 
#else
    #define DUMP_PRINT(...) do {} while(0)
    #define DUMP_TEXT(txt) do {} while(0) 
    #define IF_ASSEMBLER_DEBUG(...) do {} while(0)
#endif


static int getRegisterNum(char* reg)
{
    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
    {
        if (strcmp(reg, REGS[i].name) == 0)
        {
            DUMP_PRINT("register id = %d\n", REGS[i].id);
            return REGS[i].id;
        }
    }
    return 0;
}


static void putCommandToFile(char* name, char* reg, int num, FILE* file)
{
    DUMP_PRINT("putCommandToFile(%s, %s, %d)\n", name, reg, num);
    for (size_t i = 0; i < AMOUNT_OF_COMMANDS; i++)
    {
        if (strcmp(name, COMMANDS[i].name) == 0)
        {
            // Copy the command id.
            int command = (int) COMMANDS[i].code & COM_COMMAND_BITS;
            // Set the reg.
            if (reg != NULL)             command |= COM_REGISTER_BIT;
            // Set the imm.
            if (num != ASSEMBLER_POISON) command |= COM_IMMEDIATE_BIT; 


            if (command == COMMANDS[i].code)
            {
                                              DUMP_PRINT("Command ID is: %d\n", command);
                                              DUMP_PRINT("PUTTING TO THE FILE\n");
                                              DUMP_PRINT("name = %s\n", name);
                if (!reg)                     DUMP_PRINT("reg = %s\n", reg);
                if (num != ASSEMBLER_POISON)  DUMP_PRINT("num = %d\n", num);

                fwrite(&command, sizeof(int), 1, file);

                if (reg != NULL)
                {
                    int regId = getRegisterNum(reg);
                    fwrite(&regId, sizeof(int), 1, file);
                }
                if (num != ASSEMBLER_POISON)    
                    fwrite(&num,   sizeof(int), 1, file);
                
                return;
            }
        }
    }
    
}

static void printErrorMessage(size_t line, char* str)
{
    fprintf(stderr, "\t\tSyntax error at line %ld\n", line);
    fprintf(stderr, "\t\t\t%s", str);
}

inline static void deleteAssemblerComments(char* str)
{
    char* commentPtr = strchr(str, ';');
    if (commentPtr != NULL)
        *commentPtr = '\0';
}


AssemblerError textToAssembly(Text* txt, const char* outputFileName)
{
                                DUMP_PRINT("TEXT TO ASSEMBLY STARTED\n");
    FILE* outputFile = fopen(outputFileName, "wb");
    if (outputFile == NULL)
    {
                            DUMP_PRINT("failed to open %s", outputFileName);
        return ASSEMBLER_OPEN_FILE_ERROR;
    }

    assert(txt);
    assert(outputFile);

    for (size_t i = 0; i < txt->nLines; i++)
    {
        char* str = txt->line[i].str;

        int num = ASSEMBLER_POISON;
        char* reg = NULL;
        char* name = NULL;
                            IF_ASSEMBLER_DEBUG(fprintf(stderr, "\n"));
                            DUMP_PRINT("Analysing line num: %ld\n", i);
                            DUMP_PRINT("string: <%s>\n", str);

        size_t nWord = 0;
        char* word = strtok(str, " ");
        while (word) 
        {
            switch (nWord)
            {
                case 0:
                    if (isalpha(word[0]))
                    {
                                            DUMP_PRINT("word <%s> number %ld is a WORD\n", word, nWord);
                        name = word;
                    }
                    else
                    {
                                            DUMP_PRINT("ERROR!!!\n");
                        return ASSEMBLER_NO_SUCH_COMMAND;                
                    }
                    break;
                case 1:
                    if (isalpha(word[0]))
                    {
                        reg = word;
                                            DUMP_PRINT("word <%s> number %ld is a WORD\n", word, nWord);
                    }
                    else if (isdigit(word[0]))
                    {
                        num = atoi(word);
                                            DUMP_PRINT("word <%s> number %ld is a NUMBER %d\n", word, nWord, num);
                    }
                    else    
                    {
                                            DUMP_PRINT("ERROR: NO SUCH COMMAND WAS FOUND!\n");
                        return ASSEMBLER_NO_SUCH_COMMAND;                
                    }
                    break;
                case 2:
                    if (isdigit(word[0]))
                    {
                        num = atoi(word);
                                            DUMP_PRINT("word <%s> number %ld is a NUMBER %d\n", word, nWord, num);
                    }
                    else    
                    {
                                            DUMP_PRINT("ERROR!!!\n");
                        return ASSEMBLER_NO_SUCH_COMMAND;                
                    }
                    break;
                default:
                                        DUMP_PRINT("ERROR: TOO MANY ARGUMENTS!\n");
                    return ASSEMBLER_NO_SUCH_COMMAND;
                    break;
            }
            word = strtok(NULL, " ");
            nWord++;
        }
        putCommandToFile(name, reg, num, outputFile);
    }
    

    return ASSEMBLER_NO_ERROR;
}