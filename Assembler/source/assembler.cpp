#include "../include/assembler.h"


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


#define GENERATE_STR(x) #x,



enum CommandError 
{
    // Generate enum.
    #define DEF_ERR(name, errStr) CMD_ ## name,
    #include "../include/errors.h"
    #undef DEF_ERR
};




static const char* errorStr[] = 
{
    // Generate error string names.
    #define DEF_ERR(name, errStr) errStr,
    #include "../include/errors.h"
    #undef DEF_ERR
};



struct AssCommand
{
    char* name;
    char* reg;
    int num;

    bool hasName;
    bool hasReg;
    bool hasNum;

    size_t line;

    CommandError error;
};


// The maximum amount of commands in the output file.
// The size of the output buffer depends on this value.
const size_t MAX_NUMBER_LINE_CMD = 3;

/**
 * @brief Returns a register ID by it's name.
*/
static int getRegisterNum(char* reg);

/**
 * @brief Puts byte code into the given file.
*/
static void putBufferToFile(const Assembler* ass, FILE* outputFile);

/**
 * @brief Prints a compiling error from the `command` struct. 
 */
static void printAssError(const AssCommand* command, Assembler* ass);

/**
 * @brief Transfers the given `AssCommand` into the byte code.
 */
static void putCommandToBuffer(AssCommand* com, Assembler* ass);

/**
 * @brief Returns a pointer to the AssCommand set by the given string. May print error.
 */
static AssCommand getCommandFromLine(char* str, size_t line);


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


static void putBufferToFile(const Assembler* ass, FILE* outputFile)
{
    fwrite(ass->outputBuffer, ass->outputBufferPos, sizeof(int), outputFile);
}


static void printAssError(const AssCommand* command, Assembler* ass)
{
    assert(command);
    assert(ass);
    if (command->error == CMD_NO_ERROR) return; 
    
    int cmdNum = (int) command->error;

    switch (command->error)
    {
    case CMD_NO_ERROR:
        return;
    case CMD_INVALID_CMD_NAME:
    case CMD_TOO_MANY_ARGS:
    case CMD_INVALID_ARG:
        {
            fprintf(stderr, BOLD "%s:%lu: " RESET, ass->inputFileName, command->line);
            fprintf(stderr, RED "error:" RESET BOLD " %s\n" RESET, errorStr[cmdNum]);

            fprintf(stderr, MAGENTA "%lu | " RESET, command->line); 
            // Print the whole line.
            // This is neccessary because of the previous usage of strtok().
            size_t curStrAdress = (size_t) ass->inputText.line[command->line].str;
            size_t nextStrAdress =  (size_t) ass->inputText.line[command->line + 1].str;
            for (size_t i = 0; i < nextStrAdress - curStrAdress; i++)
            {
                char chr = * (char*) (curStrAdress + i);
                if (chr == '\0') chr = ' ';     
                fprintf(stderr, YELLOW "%c" RESET, chr);            
            }

            fprintf(stderr, YELLOW "\n\"%s\"\n" RESET, ass->inputText.line[command->line].str);
            break;
        }
    default:
        DUMP_PRINT("UNKNOWN ERROR");
        break;
    }
}


static void putCommandToBuffer(AssCommand* com, Assembler* ass)
{
    DUMP_PRINT("putCommandToFile(%s, %s, %d)\n", com->name, com->reg, com->num);
    for (size_t i = 0; i < AMOUNT_OF_COMMANDS; i++)
    {
        if (strcasecmp(com->name, COMMANDS[i].name) == 0)
        {
            // Copy the command id. 
            int commandCode = (int) COMMANDS[i].code & COM_COMMAND_BITS;
            // Set the reg.
            if (com->hasReg)             commandCode |= COM_REGISTER_BIT;
            // Set the imm.
            if (com->hasNum)             commandCode |= COM_IMMEDIATE_BIT; 


            if (commandCode == COMMANDS[i].code)
            {
                DUMP_PRINT("Command ID is: %d\n", commandCode);
                DUMP_PRINT("PUTTING TO THE BUFFER\n");
                DUMP_PRINT("name = %s\n", com->name);

                if (com->hasReg)    DUMP_PRINT("reg = %s\n", com->reg);
                if (com->hasNum)    DUMP_PRINT("num = %d\n", com->num);

                DUMP_PRINT("outputBuffer[%lu] = %d\n", ass->outputBufferPos, commandCode);
                ass->outputBuffer[ass->outputBufferPos++] = commandCode;


                if (com->hasReg)
                {
                    int regId = getRegisterNum(com->reg);
                    
                    // On error.
                    if (regId == 0)
                    {
                        com->error = CMD_INVALID_ARG;
                        printAssError(com, ass);                        
                    }

                    DUMP_PRINT("outputBuffer[%lu] = %d\n", ass->outputBufferPos, regId);
                    ass->outputBuffer[ass->outputBufferPos++] = regId;
                }

                if (com->hasNum)    
                {
                    DUMP_PRINT("outputBuffer[%lu] = %d\n", ass->outputBufferPos, com->num);
                    ass->outputBuffer[ass->outputBufferPos++] = com->num;
                }

                return;
            }
        }
    }
    com->error = CMD_INVALID_CMD_NAME;
    printAssError(com, ass);
}


static AssCommand getCommandFromLine(char* str, size_t line)
{
    AssCommand command = 
    {
        .name = NULL,
        .reg = NULL,
        .num = 0,
        
        .hasName = false,
        .hasReg = false,
        .hasNum = false,

        .line = line,

        .error = CMD_NO_ERROR,
    };

    IF_ASSEMBLER_DEBUG(fprintf(stderr, "\n"));
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
                    
                    command.name = word;
                    command.hasName = true;
                }
                else
                {
                    DUMP_PRINT("ERROR!!!\n");
                    
                    command.error = CMD_INVALID_CMD_NAME;
                    return command;                
                }
                break;
            case 1:
                if (isalpha(word[0]))
                {
                    DUMP_PRINT("word <%s> number %ld is a WORD\n", word, nWord);
                                        
                    command.reg = word;
                    command.hasReg = true;
                }
                else if (isdigit(word[0]))
                {
                    DUMP_PRINT("word <%s> number %ld is a NUMBER %d\n", word, nWord, command.num);
                    
                    command.num = atoi(word);
                    command.hasNum = true;
                }
                else    
                {
                    DUMP_PRINT("ERROR: NO SUCH COMMAND WAS FOUND!\n");

                    command.error = CMD_INVALID_ARG;
                    return command;                
                }
                break;
            case 2:
                if (isdigit(word[0]))
                {
                    DUMP_PRINT("word <%s> number %ld is a NUMBER %d\n", word, nWord, command.num);
                    
                    command.num = atoi(word);
                    command.hasNum = true;
                }
                else    
                {
                    DUMP_PRINT("ERROR!!!\n");

                    command.error = CMD_INVALID_ARG;
                    return command;                
                }
                break;
            default:
                DUMP_PRINT("ERROR: TOO MANY ARGUMENTS!\n");

                command.error = CMD_TOO_MANY_ARGS;
                return command;
        }
        word = strtok(NULL, " ");
        nWord++;
    }    
    return command;
}


inline static void deleteAssemblerComments(char* str)
{
    char* commentPtr = strchr(str, ';');
    if (commentPtr != NULL)
        *commentPtr = '\0';
}

AssemblerError textToAssembly(Assembler* ass, const char* outputFileName)
{
    DUMP_PRINT("TEXT TO ASSEMBLY STARTED\n");
    
    assert(ass);
    assert(outputFileName);
    
    FILE* outputFile = fopen(outputFileName, "wb");
    if (outputFile == NULL)
    {
        DUMP_PRINT("failed to open %s", outputFileName);
        
        return ASSEMBLER_OPEN_FILE_ERROR;
    }


    for (size_t nLine = 0; nLine < ass->inputText.nLines; nLine++)
    {
        char* str = ass->inputText.line[nLine].str;
        deleteAssemblerComments(str);

        DUMP_PRINT("Analysing line num: %ld\n", nLine);

        AssCommand curCommand = getCommandFromLine(str, nLine);
        putCommandToBuffer(&curCommand, ass);
    }
    
    putBufferToFile(ass, outputFile);
    return ASSEMBLER_NO_ERROR;
}


AssemblerError AssInit(Assembler* ass, const char* inputFile)
{
    assert(ass);
    assert(inputFile);

    // Initialize input Text from the file.  
    ParseError error = textInit(inputFile, &ass->inputText);
    if (error != PARSE_NO_ERROR)
    {
        DUMP_PRINT("ERROR! INITIALIZATION TEXT ERROR\n");
        return ASSEMBLER_PARSE_ERROR;
    }

    // Initialize output Buffer.
    int* temp = (int*) calloc(ass->inputText.nLines, MAX_NUMBER_LINE_CMD * sizeof(int));
    if (temp == NULL)
    {
        DUMP_PRINT("ERROR! ERROR ALLOCATING MEMORY\n");
        return ASSEMBLER_ALLOCATION_ERROR;
    }
    ass->outputBuffer = temp;
    ass->outputBufferPos = 0lu;
    ass->inputFileName = inputFile;

    return ASSEMBLER_NO_ERROR;
}


AssemblerError AssDtor(Assembler* ass)
{
    assert(ass);
    assert(ass->outputBuffer);

    textDtor(&ass->inputText);
    free(ass->outputBuffer);

    return ASSEMBLER_NO_ERROR;
}