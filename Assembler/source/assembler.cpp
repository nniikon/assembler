#include "../include/assembler.h"


#define ASSEMBLER_DEBUG   

#ifdef ASSEMBLER_DEBUG
    #define DUMP_PRINT(...)\
    do\
    {\
        fprintf(stderr, "file: %s line: %d\t\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
    } while (0)

    #define DUMP_PRINT_CYAN(...)\
    do\
    {\
        fprintf(stderr, CYAN "file: %s line: %d\t\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
        fprintf(stderr, RESET);\
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


static const char* errorStr[] = 
{
    // Generate error string names.
    #define DEF_ERR(name, errStr) errStr,
    #include "../include/errors.h"
    #undef DEF_ERR
};


struct AssCommand
{
    bool hasReg;
    bool hasNum;
    bool hasLabel;
    bool hasMem;

    size_t cmdID;

    size_t line;

    CommandError error;
};

// Could be any number.
// Doesn't really matter.
const size_t LABLE_POISON = -1;

const int NUMBER_OF_COMPILATIONS = 2;


/**
 * @brief Puts byte code into the given file.
*/
static void putBufferToFile(const Assembler* ass, FILE* outputFile);

static void setLabels(Assembler* ass);

static CommandError checkCommandCorrectness(AssCommand* command, Assembler* ass, size_t pos);

AssemblerError textToAssembly(Assembler* ass, FILE* outputFile);

static CommandError putCommandNameToBuffer(const char** str, Assembler* ass, AssCommand* cmd);

static void setLabelName(const char* str, Assembler* ass);

static bool putRegToBuffer(const char** str, Assembler* ass);

static bool putLabelToBuffer(const char** str, Assembler* ass);

static bool putNumberToBuffer(const char** str, Assembler* ass);

static CommandError setArgs(const char** str, Assembler* ass, AssCommand* command);

static void adjustCommandCode(AssCommand* command, Assembler* ass, size_t pos);


static AssemblerError pushAssErrArray(AssErrorArray* errArr, AssError* error);

static AssemblerError AssErrorInit(AssErrorArray* errArr);

static void printAssError(Assembler* ass);




static void putBufferToFile(const Assembler* ass, FILE* outputFile)
{
    fwrite(ass->outputBuffer, ass->outputBufferPos, sizeof(char), outputFile);
}



static CommandError putCommandNameToBuffer(const char** str, Assembler* ass, AssCommand* cmd)
{
    assert(str);
    assert(ass);

    DUMP_PRINT("Setting command name:\n");
    size_t size = 0;
    char delim = getWordSize(&size, *str, " ");

    DUMP_PRINT("Command name length: <%zu>, name: <%.*s>\n", size, (int)size, *str);

    for (size_t com = 0; com < AMOUNT_OF_COMMANDS; com++)
    {
        if (strncasecmp(*str, COMMANDS[com].name, size) == 0)
        {
            ass->outputBuffer[ass->outputBufferPos] = (COMMANDS[com].code & COM_COMMAND_BITS);
            ass->outputBufferPos += sizeof(uint8_t); 

            DUMP_PRINT("Command name: <%s>\n", COMMANDS[com].name);
            DUMP_PRINT("Command code: <%d>\n", COMMANDS[com].code & COM_COMMAND_BITS);
            
            cmd->cmdID = com;

            if (delim == '\0') 
                *str = NULL;
            else
                moveToNextWord(str, " +");

            return CMD_NO_ERROR;
        }
    }
    return CMD_INVALID_CMD_NAME;
}


static void setLabelName(const char* str, Assembler* ass)
{
    assert(str);
    assert(ass);

    DUMP_PRINT("Setting label name:\n");
    size_t size = 0;
    getWordSize(&size,  str, ":");

    ass->labels[ass->emptyLabel].adress = (int) ass->outputBufferPos;
    ass->labels[ass->emptyLabel].len = size;
    ass->labels[ass->emptyLabel].name = str;

    DUMP_PRINT("Label   size: <%zu>\n", ass->labels[ass->emptyLabel].len);
    DUMP_PRINT("Label   name: <%.*s>\n", (int) ass->labels[ass->emptyLabel].len, ass->labels[ass->emptyLabel].name); 
    DUMP_PRINT("Label adress: <%d>\n", ass->labels[ass->emptyLabel].adress);
    
    (ass->emptyLabel)++;
}


// This function is needed to get the label names.
// And not to set the adresses.
// This function also sets outputBufferPos to zero.
static void setLabels(Assembler* ass)
{
    assert(ass);

    IF_ASSEMBLER_DEBUG(putc('\n', stderr));
    DUMP_PRINT("setLabels started\n");
    for (size_t line = 0; line < ass->inputText.nLines; line++)
    {   
        char* str = ass->inputText.line[line].str;

        ass->outputBufferPos = LABLE_POISON;

        if (strchr(str, ':') != NULL)
        {
            setLabelName(str, ass);

            continue;
        }

        ass->outputBufferPos = 0;
    }
    DUMP_PRINT("setLabels successful\n");
    
}


static bool putRegToBuffer(const char** str, Assembler* ass)
{
    size_t size = 0;
    char del = getWordSize(&size, *str, " ]+");
    
    if (size != REGISTER_LENGTH) 
        return false;

    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
    {
        if (strncasecmp(*str, REGS[i].name, size) == 0)
        {
            DUMP_PRINT("\"%.*s\"\n", (int)size, *str);
            DUMP_PRINT("Register <%s>\n", REGS[i].name);
            DUMP_PRINT("id = <%d>\n", (int) REGS[i].id);

            ass->outputBuffer[ass->outputBufferPos] = REGS[i].id;
            ass->outputBufferPos += sizeof(uint8_t);

            if (del == '\0' || del == ']')
                *str = NULL;
            else
                moveToNextWord(str, " +]");

            return true;
        }
    }
    return false;
}


static bool putLabelToBuffer(const char** str, Assembler* ass)
{
    size_t size = 0;
    char del = getWordSize(&size, *str, " ]+");

    for (size_t i = 0; i < MAX_NUMBER_OF_LABELS; i++)
    {
        if (size != ass->labels[i].len)
            continue;
        
        if (strncasecmp(*str, ass->labels[i].name, size) == 0)
        {
            DUMP_PRINT("\"%.*s\"\n", (int)size, *str);
            DUMP_PRINT("Label <%s>\n", ass->labels[i].name);
            DUMP_PRINT("Adress = <%d>\n", (int)ass->labels[i].adress);

            memcpy(ass->outputBuffer + ass->outputBufferPos, &(ass->labels[i].adress), sizeof(int));
            ass->outputBufferPos += sizeof(int);

            if (del == '\0' || del == ']')
                *str = NULL;
            else
                moveToNextWord(str, " +]");
            return true;
        }
    }
    return false;
}


static bool putNumberToBuffer(const char** str, Assembler* ass)
{
    size_t size = 0;
    char del = getWordSize(&size, *str, " ]+");

    for (size_t i = 0; i < size; i++)
    {
        if (isdigit((*str)[i]) == 0)
            return false;
    }
    int num = atoi(*str) * FLOATING_POINT_COEFFICIENT;

    DUMP_PRINT("\"%.*s\"\n", (int)size, *str);
    DUMP_PRINT("number: <%d>\n", num);

    memcpy(ass->outputBuffer + ass->outputBufferPos, &num, sizeof(int));
    ass->outputBufferPos += sizeof(int);

    if (del == '\0' || del == ']')
        *str = NULL;
    else
        moveToNextWord(str, " +]");

    return true;
}


static CommandError setArgs(const char** str, Assembler* ass, AssCommand* command)
{
    if (*str == NULL) return CMD_NO_ERROR;

    assert(str);
    assert(ass);
    assert(command);

    DUMP_PRINT("setArgs started with <%s>\n", *str);

    // If the first character is a letter, treat it as a word. 
    if (isalpha((*str)[0]))
    {
        if (putRegToBuffer(str, ass))
        {
            command->hasReg = true;
            return setArgs(str, ass, command);
        }    
        else if (putLabelToBuffer(str, ass))
        {
            command->hasLabel = true;
            return setArgs(str, ass, command);
        }    
        else 
        {
            return CMD_INVALID_ARG;
        }
    }
    else if ((*str)[0] == '[')
    {
        command->hasMem = true;
        (*str)++;
        return setArgs(str, ass, command);
    }
    else if (isdigit((*str)[0]))
    {
        if (putNumberToBuffer(str, ass)) 
        {
            command->hasNum = true;
            return setArgs(str, ass, command);
        }
        else
        {
            return CMD_INVALID_ARG;
        }
    }
    else
    {
        return CMD_INVALID_ARG;
    }

    DUMP_PRINT("setArgs successful\n");
    return CMD_NO_ERROR;
}


static void adjustCommandCode(AssCommand* command, Assembler* ass, size_t pos)
{
    assert(command);
    assert(ass);

    DUMP_PRINT("Started adjusting command code\n");

    if (command->hasLabel) ass->outputBuffer[pos] |= COM_IMMEDIATE_BIT;
    if (command->hasNum  ) ass->outputBuffer[pos] |= COM_IMMEDIATE_BIT;
    if (command->hasMem  ) ass->outputBuffer[pos] |= COM_MEMORY_BIT;
    if (command->hasReg  ) ass->outputBuffer[pos] |= COM_REGISTER_BIT;

    if (command->hasLabel) DUMP_PRINT("add imm(label) bit\n");
    if (command->hasNum  ) DUMP_PRINT("add imm(number) bit\n");
    if (command->hasMem  ) DUMP_PRINT("add ram bit\n");
    if (command->hasReg  ) DUMP_PRINT("add reg bit\n");

    DUMP_PRINT("Command code: %d\n", ass->outputBuffer[pos]);

    DUMP_PRINT("Adjustment success\n");
}


AssemblerError textToAssembly(Assembler* ass, FILE* outputFile)
{
    assert(ass);
    assert(outputFile);
    
    DUMP_PRINT("TEXT TO ASSEMBLY STARTED\n");

    for (size_t line = 0; line < ass->inputText.nLines; line++)
    {   
        deleteMeaninglessSpaces(ass->inputText.line[line].str);

        const char* str = ass->inputText.line[line].str;
        if (str[0] == '\0' || str[0] == '\n') continue; // TODO: fix

        IF_ASSEMBLER_DEBUG(fputc('\n', stderr));
        DUMP_PRINT("Analysing line num: %ld\n", line);
        DUMP_PRINT("<%s>\n", str);

        AssCommand command = 
        {
            .hasReg = false,
            .hasNum = false,
            .hasLabel = false,
            .hasMem = false,

            .cmdID = 0,

            .line = line,

            .error = CMD_NO_ERROR,
        };
        size_t cmdNameBufferPos = ass->outputBufferPos;

        CommandError err = CMD_NO_ERROR;

        // If the line has ':' in it...
        if (strchr(str, ':') != NULL)
        {
            setLabelName(str, ass); // Set the label, ...
            continue;
        }
        // Otherwise, ...
        else
        {
            // Threat the first word as a command name.
            err = putCommandNameToBuffer(&str, ass, &command);
            if (err != CMD_NO_ERROR)
            {
                AssError tempError = {err, line};
                pushAssErrArray(&(ass->errorArray), &tempError);
                continue;
            }
        }
        // Set the args.
        err = setArgs(&str, ass, &command);
        if (err != CMD_NO_ERROR)
        {
            AssError tempError = {err, line};
            pushAssErrArray(&(ass->errorArray), &tempError);
            continue;
        }
        adjustCommandCode(&command, ass, cmdNameBufferPos);

        err = checkCommandCorrectness(&command, ass, cmdNameBufferPos);
        if (err != CMD_NO_ERROR)
        {
            AssError tempError = {err, line};
            pushAssErrArray(&(ass->errorArray), &tempError);
            continue;
        }
    }
    
    return ASSEMBLER_NO_ERROR;
}


void assembly(Assembler* ass, FILE* outputFile)
{
    setLabels(ass);
    for (int i = 0; i < NUMBER_OF_COMPILATIONS; i++)
    {
        ass->errorArray.emptyIndex = 0;
        ass->emptyLabel = 0;
        ass->outputBufferPos = 0;
        textToAssembly(ass, outputFile);
    }
    putBufferToFile(ass, outputFile);
    printAssError(ass);
    return;
}


static AssemblerError pushAssErrArray(AssErrorArray* errArr, AssError* error)
{
    DUMP_PRINT_CYAN("Pushing error\n");
    if ((errArr->emptyIndex + 1) == errArr->capacity)
    {
        AssError* temp = (AssError*) realloc(errArr->err, errArr->capacity * 2 * sizeof(AssError));
        if (temp == NULL)
        {
            DUMP_PRINT("Error allocating memory\n");
            return ASSEMBLER_ALLOCATION_ERROR;
        }
        errArr->err = temp; 
        errArr->capacity *= 2;
    }

    errArr->err[errArr->emptyIndex].err = error->err;
    errArr->err[errArr->emptyIndex].line = error->line;
    errArr->emptyIndex++;
    DUMP_PRINT_CYAN("Pushing error success\n");

    return ASSEMBLER_NO_ERROR;
}


static AssemblerError AssErrorInit(AssErrorArray* errArr)
{
    assert(errArr);

    AssError* temp = (AssError*) calloc(DEFAULT_SIZE_OF_ERROR_ARR, sizeof(AssError));
    if (temp == NULL)
    {
        DUMP_PRINT("Error allocating memory\n");
        return ASSEMBLER_ALLOCATION_ERROR;
    }

    errArr->err = temp;
    errArr->capacity = DEFAULT_SIZE_OF_ERROR_ARR;
    errArr->emptyIndex = 0;

    return ASSEMBLER_NO_ERROR;
}


AssemblerError AssInit(Assembler* ass, const char* inputFile)
{
    assert(ass);
    assert(inputFile);

    DUMP_PRINT("AssInit started\n");

    // Initialize input Text from the file.  
    ParseError error = textInit(inputFile, &ass->inputText);
    if (error != PARSE_NO_ERROR)
    {
        DUMP_PRINT("ERROR! INITIALIZATION TEXT ERROR\n");
        return ASSEMBLER_PARSE_ERROR;
    }

    // Initialize output Buffer.
    uint8_t* temp = (uint8_t*) calloc(ass->inputText.nLines, MAX_NUMBER_LINE_CMD * sizeof(int));
    if (temp == NULL)
    {
        DUMP_PRINT("ERROR! ERROR ALLOCATING MEMORY\n");
        return ASSEMBLER_ALLOCATION_ERROR;
    }
    ass->outputBuffer = temp;
    ass->outputBufferPos = 0lu;

    ass->inputFileName = inputFile;

    ass->emptyLabel = 0;

    // Initialize error array.
    AssemblerError err = ASSEMBLER_NO_ERROR;
    err = AssErrorInit(&(ass->errorArray));
    if (err != ASSEMBLER_NO_ERROR)
    {
        return err;
    }

    DUMP_PRINT("AssInit success\n");


    return ASSEMBLER_NO_ERROR;
}


AssemblerError AssDtor(Assembler* ass)
{
    assert(ass);
    assert(ass->outputBuffer);

    textDtor(&ass->inputText);
    free(ass->errorArray.err);
    free(ass->outputBuffer);

    return ASSEMBLER_NO_ERROR;
}


static void printAssError(Assembler* ass)
{
    const char* fileName = ass->inputFileName;
    for (size_t errorID = 0; errorID < ass->errorArray.emptyIndex; errorID++)
    {
        AssError cmd = ass->errorArray.err[errorID];

        fprintf(stderr, "%s(%zu): " MAGENTA "%s" RESET "\n", fileName, cmd.line + 1, errorStr[(int)cmd.err]);       
        fprintf(stderr, "\t %zu| %s\n\n", cmd.line + 1, ass->inputText.line[cmd.line].str);
    }
    
}


static CommandError checkCommandCorrectness(AssCommand* cmd, Assembler* ass, size_t pos)
{   
    DUMP_PRINT("Checking command correctness\n");
    size_t nBits = sizeof(uint8_t) * CHAR_BIT;
    uint8_t cmdCode = ass->outputBuffer[pos];
    
    int    sumOfBits = 0;
    int refSumOfBits = 0; 

    for (int bitPos = (int) (nBits - 1); bitPos >= (int) NUMBER_OF_CMD_BITS; bitPos--)
    {
        uint8_t bit    = (cmdCode                   >> bitPos) & 1; // Get the command bit.
        uint8_t refBit = (COMMANDS[cmd->cmdID].code >> bitPos) & 1; // Get the reference bit.

        sumOfBits += bit;
        refSumOfBits += refBit;

        // If the bit is unexpected...
        if (refBit == 0 && bit == 1) 
        {
            // TODO: better error recognision.
            DUMP_PRINT("Incorrect bit in encountered\n");
            return CMD_INVALID_ARG; // ... return error.
        }
    }
    if (sumOfBits == 0 && refSumOfBits != 0)
    {
        DUMP_PRINT("Too few bits encountered\n");
        return CMD_TOO_FEW_ARGS;
    }
    
    DUMP_PRINT("Command is correct\n");
    return CMD_NO_ERROR;
}