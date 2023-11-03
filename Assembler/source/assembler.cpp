#include "../include/assembler.h"

#define DUMP_DEBUG   
#include "../../lib/dump.h"



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
const int LABEL_POISON = -1;

const int NUMBER_OF_COMPILATIONS = 2;

const char* DELIMS = "\t +];";

/**
 * @brief Puts byte code into the given file.
*/
static AssemblerError putBufferToFile(const Assembler* ass, FILE* outputFile);

static CommandError checkCommandCorrectness(AssCommand* command, Assembler* ass, size_t pos);

AssemblerError textToAssembly(Assembler* ass, FILE* outputFile, const bool isLastAssembly);

static CommandError putCommandNameToBuffer(const char** str, Assembler* ass, AssCommand* cmd);

static CommandError setLabelName(const char* str, Assembler* ass);

static bool putRegToBuffer(const char** str, const size_t size, Assembler* ass);

static bool putLabelToBuffer(const char** str, const size_t size, Assembler* ass);

static bool putNumberToBuffer(const char** str, const size_t size, Assembler* ass);

static CommandError setArgs(const char** str, Assembler* ass, AssCommand* command);

static void adjustCommandCode(AssCommand* command, Assembler* ass, size_t pos);


static AssemblerError putBufferToFile(const Assembler* ass, FILE* outputFile)
{
    size_t refSize = fwrite(ass->outputBuffer, sizeof(char), ass->outputBufferPos, outputFile);
    if (refSize != ass->outputBufferPos)
        return ASSEMBLER_FWRITE_ERROR;
    return ASSEMBLER_NO_ERROR;
}


static void moveToNextArgument(const char** str, size_t size, const char del)
{
    if (del == '\0' || del == ']')
        *str = NULL;
    else
        moveToNextWord(str, size, DELIMS);
}


static CommandError putCommandNameToBuffer(const char** str, Assembler* ass, AssCommand* cmd)
{
    assert(str);
    assert(ass);

    DUMP_PRINT("Setting command name:\n");
    size_t size = 0;
    char del = getWordSize(&size, *str, " ");

    DUMP_PRINT("Command name length: <%zu>, name: <%.*s>\n", size, (int)size, *str);

    for (size_t cmdID = 0; cmdID < AMOUNT_OF_COMMANDS; cmdID++)
    {
        if (strncasecmp(*str, COMMANDS[cmdID].name, size) == 0)
        {
            ass->outputBuffer[ass->outputBufferPos] = (COMMANDS[cmdID].code & CMD_COMMAND_BITS); // https://www.youtube.com/watch?v=UMfMekip6y8
            ass->outputBufferPos += sizeof(uint8_t); 

            DUMP_PRINT("Command name: <%s>\n", COMMANDS[cmdID].name);
            DUMP_PRINT("Command code: <%d>\n", COMMANDS[cmdID].code & CMD_COMMAND_BITS);

            cmd->cmdID = cmdID;

            moveToNextArgument(str, size, del);

            return CMD_NO_ERROR;
        }
    }
    return CMD_INVALID_CMD_NAME;
}


static CommandError setLabelName(const char* str, Assembler* ass)
{
    assert(str);
    assert(ass);

    DUMP_PRINT("Setting label name:\n");

    if (ass->labelArr.emptyLabel >= MAX_NUMBER_OF_LABELS)
    {
        fprintf(stdout, "You wrote more than %zu labels and hit the limit.\n", MAX_NUMBER_OF_LABELS);
        fprintf(stdout, "You can change this value in ./Assembler/source/assembler.cpp\n.");
        return CMD_HIT_MAX_LABLES;
    }

    if (chrAppearsMoreTimes(str, ':', 1))
        return CMD_TOO_MANY_COLONS;

    size_t size = 0;
    getWordSize(&size, str, ":");

    // Check for repeated labels.
    for (size_t i = 0; i < ass->labelArr.emptyLabel; i++)
    {
        if (size != ass->labelArr.labels[i].len)
            continue;
        if (strncmp(str, ass->labelArr.labels[i].name, size) == 0)
        {
            DUMP_PRINT("Error. This label already exists.\n");
            return CMD_REPEATED_LABEL;
        }
    }

    // Set the values.
    if (ass->labelArr.emptyLabel > ass->labelArr.nLabels)
        ass->labelArr.nLabels = ass->labelArr.emptyLabel + 1;

    ass->labelArr.labels[ass->labelArr.emptyLabel].adress = (int) ass->outputBufferPos;
    ass->labelArr.labels[ass->labelArr.emptyLabel].len = size;
    ass->labelArr.labels[ass->labelArr.emptyLabel].name = str;

    DUMP_PRINT("Label   size: <%zu>\n",        ass->labelArr.labels[ass->labelArr.emptyLabel].len);
    DUMP_PRINT("Label   name: <%.*s>\n", (int) ass->labelArr.labels[ass->labelArr.emptyLabel].len,
                                               ass->labelArr.labels[ass->labelArr.emptyLabel].name); 
    DUMP_PRINT("Label adress: <%d>\n",         ass->labelArr.labels[ass->labelArr.emptyLabel].adress);

    (ass->labelArr.emptyLabel)++;

    return CMD_NO_ERROR;
}


static bool putRegToBuffer(const char** str, const size_t size, Assembler* ass)
{
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

            return true;
        }
    }
    return false;
}


static bool putLabelToBuffer(const char** str, const size_t size, Assembler* ass)
{
    assert(str);
    assert(ass);

    DUMP_PRINT("Putting label to the buffer\n");
    for (size_t i = 0; i < ass->labelArr.nLabels; i++)
    {
        if (size != ass->labelArr.labels[i].len)
            continue;

        if (strncasecmp(*str, ass->labelArr.labels[i].name, size) == 0)
        {
            DUMP_PRINT("\"%.*s\"\n", (int)size, *str);
            DUMP_PRINT("Label <%s>\n", ass->labelArr.labels[i].name);
            DUMP_PRINT("Adress = <%d>\n", (int)ass->labelArr.labels[i].adress);

            memcpy(ass->outputBuffer + ass->outputBufferPos, &(ass->labelArr.labels[i].adress), sizeof(int));
            ass->outputBufferPos += sizeof(int);
            DUMP_PRINT("Putting label success\n");
            return true;
        }
    }

    DUMP_PRINT("No such label was found\n");
    memcpy(ass->outputBuffer + ass->outputBufferPos, &LABEL_POISON, sizeof(int));
    ass->outputBufferPos += sizeof(int);

    return false;
}

static inline bool isNumber(char chr) 
{
    return (chr >= '0' && chr <= '9') || chr == '+' || chr == '-' || chr == '.';
}

static bool putNumberToBuffer(const char** str, const size_t size, Assembler* ass)
{
    DUMP_PRINT("Putting number to buffer.\n");
    for (size_t i = 0; i < size; i++)
    {
        if (isNumber((*str)[i]) == 0)
            return false;
    }
    int num = atoi(*str) * FLOATING_POINT_COEFFICIENT;

    DUMP_PRINT("\"%.*s\"\n", (int)size, *str);
    DUMP_PRINT("number: <%d>\n", num);

    memcpy(ass->outputBuffer + ass->outputBufferPos, &num, sizeof(int));
    ass->outputBufferPos += sizeof(int);

    return true;
}


static CommandError setArgs(const char** str, Assembler* ass, AssCommand* command)
{
    assert(str);

    if (*str == NULL) return CMD_NO_ERROR;

    assert(ass);
    assert(command);

    DUMP_PRINT("setArgs started with <%s>\n", *str);

    size_t size = 0;
    char del = getWordSize(&size, *str, DELIMS);

    // If the first character is a letter, treat it as a word. 
    if (isalpha((*str)[0]))
    {
        if (putRegToBuffer(str, size, ass))
        {
            command->hasReg = true;
            moveToNextArgument(str, size, del);
            return setArgs(str, ass, command);
        }
        else if (putLabelToBuffer(str, size, ass))
        {
            command->hasLabel = true;
            moveToNextArgument(str, size, del);
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
    else if (isNumber((*str)[0]))
    {
        if (putNumberToBuffer(str, size, ass)) 
        {
            command->hasNum = true;
            moveToNextArgument(str, size, del);
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

    if (command->hasLabel) ass->outputBuffer[pos] |= CMD_IMMEDIATE_BIT;
    if (command->hasNum  ) ass->outputBuffer[pos] |= CMD_IMMEDIATE_BIT;
    if (command->hasMem  ) ass->outputBuffer[pos] |= CMD_MEMORY_BIT;
    if (command->hasReg  ) ass->outputBuffer[pos] |= CMD_REGISTER_BIT;

    if (command->hasLabel) DUMP_PRINT("add imm(label) bit\n");
    if (command->hasNum  ) DUMP_PRINT("add imm(number) bit\n");
    if (command->hasMem  ) DUMP_PRINT("add ram bit\n");
    if (command->hasReg  ) DUMP_PRINT("add reg bit\n");

    DUMP_PRINT("Command code: %d\n", ass->outputBuffer[pos]);

    DUMP_PRINT("Adjustment success\n");
}


#define CHECK_PUSH_CMD_ERR(err, isLastAssembly)\
    if ((err) != CMD_NO_ERROR && (isLastAssembly))\
    {\
        AssError tempError = {err, line, ass->inputFileName};\
        pushAssErrArray(&(ass->errorArray), &tempError);\
        continue;\
    }


AssemblerError textToAssembly(Assembler* ass, FILE* outputFile, const bool isLastAssembly)
{
    assert(ass);
    assert(outputFile);
    DUMP_PRINT("TEXT TO ASSEMBLY STARTED\n");

    for (size_t line = 0; line < ass->inputText.nLines; line++)
    {
        // Temporary null-terminate the comments.
        char* commentPtr = strchr(ass->inputText.line[line].str, COMMENTS_CHR);
        bool hasComments = false;
        if (commentPtr != NULL)
        {
            hasComments = true;
            *commentPtr = '\0';
        }

        const char* str = ass->inputText.line[line].str;

        // Skip insignificant characters.
        moveToNextWord(&str, 0, DELIMS);
        if (str == NULL) continue;

        IF_DUMP_DEBUG(fputc('\n', stderr));
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

        // Save the command name position.
        size_t cmdNameBufferPos = ass->outputBufferPos;

        CommandError err = CMD_NO_ERROR;

        // If the line has ':' in it...
        if (strchr(str, ':') != NULL)
        {
            err = setLabelName(str, ass); // Set the label, ...
            CHECK_PUSH_CMD_ERR(err, isLastAssembly);
            continue;
        }
        // else))), ...
        else
        {
            // Threat the first word as a command name.
            err = putCommandNameToBuffer(&str, ass, &command);
            CHECK_PUSH_CMD_ERR(err, isLastAssembly);
        }
        // Set the args.
        err = setArgs(&str, ass, &command);
        CHECK_PUSH_CMD_ERR(err, isLastAssembly);

        adjustCommandCode(&command, ass, cmdNameBufferPos);

        err = checkCommandCorrectness(&command, ass, cmdNameBufferPos);
        CHECK_PUSH_CMD_ERR(err, isLastAssembly);

        if (hasComments)
            *commentPtr = COMMENTS_CHR;
    }
    return ASSEMBLER_NO_ERROR;
}


void assembly(Assembler* ass, FILE* outputFile)
{
    for (int i = 0; i < NUMBER_OF_COMPILATIONS; i++)
    {
        DUMP_PRINT("File name: %s\n", ass->inputFileName);
        ass->errorArray.emptyIndex = 0;
        ass->labelArr.emptyLabel = 0;
        ass->outputBufferPos = 0;
        if (i + 1 == NUMBER_OF_COMPILATIONS)
            textToAssembly(ass, outputFile, true);
        else
            textToAssembly(ass, outputFile, false);
    }
    putBufferToFile(ass, outputFile);
    return;
}


AssemblerError assInit(Assembler* ass, const char* inputFile)
{
    assert(ass);
    assert(inputFile);

    DUMP_PRINT("assInit started\n");

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

    // Initialize LabelArr
    ass->labelArr.emptyLabel = 0;
    ass->labelArr.nLabels    = 0;
    Label* tempLabel = (Label*) calloc(MAX_NUMBER_OF_LABELS, sizeof(Label));
    if (tempLabel == NULL)
    {
        free(temp);
        DUMP_PRINT("ERROR! ERROR ALLOCATING MEMORY\n");
        return ASSEMBLER_ALLOCATION_ERROR;
    }
    ass->labelArr.labels = tempLabel;


    // Initialize error array.
    AssemblerError err = ASSEMBLER_NO_ERROR;
    err = AssErrorInit(&(ass->errorArray));
    if (err != ASSEMBLER_NO_ERROR)
    {
        return err;
    }

    DUMP_PRINT("assInit success\n");


    return ASSEMBLER_NO_ERROR;
}


AssemblerError assDtor(Assembler* ass)
{
    assert(ass);
    assert(ass->outputBuffer);

    textDtor(&ass->inputText);
    free(ass->outputBuffer);
    free(ass->labelArr.labels);
    AssErrorDtor(&ass->errorArray);

    return ASSEMBLER_NO_ERROR;
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