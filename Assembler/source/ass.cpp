#include "../include/ass.h"

#define DUMP_DEBUG   
#include "../../lib/dump.h"



struct AssCommand
{
    bool hasReg;
    bool hasNum;
    bool hasLabel;
    bool hasMem;

    size_t cmdID;

    size_t nBrackets;
    size_t line;

    CommandError error;
};

// Could be any number.
// Doesn't really matter.
const int LABEL_POISON = -1;

const int NUMBER_OF_COMPILATIONS = 2;

const char* ARG_DELIMS = "\t +]";

/**
 * @brief Puts byte code into the given file.
*/
static AssemblerError putBufferToFile(const Assembler* ass);

static CommandError checkCommandCorrectness(AssCommand* command, Assembler* ass, size_t pos);

AssemblerError textToAssembly(Assembler* ass, const bool isLastAssembly);

static CommandError putCommandNameToBuffer(const char** str, Assembler* ass, AssCommand* cmd);

static CommandError setLabelName(const char* str, Assembler* ass);

static bool tryPuttingRegToBuffer(const char** str, const size_t size, Assembler* ass);

static bool tryPuttingLabelToBuffer(const char** str, const size_t size, Assembler* ass);

static bool tryPuttingNumberToBuffer(const char** str, const size_t size, Assembler* ass);

static CommandError setArgs(const char** str, Assembler* ass, AssCommand* command);

static void adjustCommandCode(AssCommand* command, Assembler* ass, size_t pos);


static AssemblerError putBufferToFile(const Assembler* ass)
{
    size_t refSize = fwrite(ass->outputBuffer, sizeof(char), ass->outputBufferPos, ass->outputFile);
    if (refSize != ass->outputBufferPos)
        return ASSEMBLER_FWRITE_ERROR;
    return ASSEMBLER_NO_ERROR;
}


static void moveToNextArgument(const char** str, size_t size, const char del)
{
    if (del == '\0' || del == ']')
        *str = NULL;
    else
        moveToNextWord(str, size, ARG_DELIMS);
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
            ass->outputBuffer[ass->outputBufferPos] = (COMMANDS[cmdID].code & CMD_COMMAND_BITS);
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

    if (nChrInLine(str, ':') > 1)
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
    if (ass->labelArr.emptyLabel >= ass->labelArr.nLabels)
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


static bool tryPuttingRegToBuffer(const char** str, const size_t size, Assembler* ass)
{
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


static bool tryPuttingLabelToBuffer(const char** str, const size_t size, Assembler* ass)
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
    return (isdigit(chr) || chr == '+' || chr == '-' || chr == '.');
}

static bool tryPuttingNumberToBuffer(const char** str, const size_t size, Assembler* ass)
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
    assert(ass);
    assert(command);

    // If the recursion is over.
    if (*str == NULL) return CMD_NO_ERROR;


    DUMP_PRINT("setArgs started with <%s>\n", *str);

    size_t size = 0;
    moveToNextWord(str, 0, ARG_DELIMS);
    char del = getWordSize(&size, *str, ARG_DELIMS);

    // If the first character is a letter, treat it as a word. 
    if (isalpha((*str)[0]))
    {
        if (tryPuttingRegToBuffer(str, size, ass))
        {
            command->hasReg = true;
            moveToNextArgument(str, size, del);
            return setArgs(str, ass, command);
        }
        else if (tryPuttingLabelToBuffer(str, size, ass))
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
        command->nBrackets++;
        return setArgs(str, ass, command);
    }
    else if (isNumber((*str)[0]))
    {
        if (tryPuttingNumberToBuffer(str, size, ass)) 
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


#define CHECK_PUSH_CMD_ERR(err)\
    do\
    {\
        if ((err) != CMD_NO_ERROR && (isLastAssembly))\
        {\
            AssError tempError = {err, line, ass->inputFileName};\
            pushAssErrArray(&(ass->errorArray), &tempError);\
            continue;\
        }\
    } while(0)


AssemblerError textToAssembly(Assembler* ass, const bool isLastAssembly)
{
    assert(ass);
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
        moveToNextWord(&str, 0, ARG_DELIMS);
        if (str == NULL) continue;

        // Check the correctness of brackets.
        if (nChrInLine(str, '[') != nChrInLine(str, ']'))
            CHECK_PUSH_CMD_ERR(CMD_MISMATCHED_BRACKET);

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
            .nBrackets = 0,

            .line = line,

            .error = CMD_NO_ERROR,
        };

        // Save the command name position.
        size_t cmdNameBufferPos = ass->outputBufferPos;

        CommandError err = CMD_NO_ERROR;

        // If the line has ':' in it...
        if (strchr(str, ':') != NULL)
        {
            err = setLabelName(str, ass); // Set the label.
            CHECK_PUSH_CMD_ERR(err);
            continue;
        }
        else
        {
            // Threat the first word as a command name.
            err = putCommandNameToBuffer(&str, ass, &command);
            CHECK_PUSH_CMD_ERR(err);
        }
        // Set the args.
        err = setArgs(&str, ass, &command);
        CHECK_PUSH_CMD_ERR(err);

        adjustCommandCode(&command, ass, cmdNameBufferPos);

        err = checkCommandCorrectness(&command, ass, cmdNameBufferPos);
        CHECK_PUSH_CMD_ERR(err);

        if (hasComments)
            *commentPtr = COMMENTS_CHR;
    }
    return ASSEMBLER_NO_ERROR;
}


AssemblerError assembly(Assembler* ass)
{
    assert(ass);
    assert(ass->outputFile);
    for (int i = 0; i < NUMBER_OF_COMPILATIONS; i++)
    {
        DUMP_PRINT("File name: %s\n", ass->inputFileName);
        ass->errorArray.emptyIndex = 0;
        ass->labelArr.emptyLabel = 0;
        ass->outputBufferPos = 0;

        AssemblerError err = ASSEMBLER_NO_ERROR;
        if (i + 1 == NUMBER_OF_COMPILATIONS)
        {
            err = textToAssembly(ass, true);
        }
        else
        {
            err = textToAssembly(ass, false);
        }

        if (err != ASSEMBLER_NO_ERROR)
            return err;
    }
    putBufferToFile(ass);
    return ASSEMBLER_NO_ERROR;
}


AssemblerError assInit(Assembler* ass, const char* inputFileName, const char* outputFileName)
{
    assert(ass);
    assert(inputFileName);

    DUMP_PRINT("assInit started\n");

    // Initialize input Text from the file. 
    FileError error = textInit(inputFileName, &ass->inputText);
    if (error != PARSE_NO_ERROR)
    {
        fprintf(stdout, "Initializing text error\n");
        return ASSEMBLER_PARSE_ERROR;
    }

    // Open the output file.
    ass->outputFile = fopen(outputFileName, "wb");
    if (ass->outputFile == NULL)
    {
        textDtor(&ass->inputText);
        fprintf(stdout, "failed to open %s\n", outputFileName);
        return ASSEMBLER_OPEN_FILE_ERROR;
    }

    // Initialize output Buffer.
    uint8_t* temp = (uint8_t*) calloc(ass->inputText.nLines, MAX_NUMBER_LINE_CMD * sizeof(int));
    if (temp == NULL)
    {
        fprintf(stdout, "Allocating memory error.\n");

        fclose(ass->outputFile);
        textDtor(&ass->inputText);

        return ASSEMBLER_ALLOCATION_ERROR;
    }
    ass->outputBuffer = temp;
    ass->outputBufferPos = 0lu;

    ass->inputFileName = inputFileName;

    // Initialize LabelArr
    ass->labelArr.emptyLabel = 0;
    ass->labelArr.nLabels    = 0;
    Label* tempLabel = (Label*) calloc(MAX_NUMBER_OF_LABELS, sizeof(Label));
    if (tempLabel == NULL)
    {
        fprintf(stdout, "Allocating memory error.\n");

        free(temp);
        fclose(ass->outputFile);
        textDtor(&ass->inputText);

        return ASSEMBLER_ALLOCATION_ERROR;
    }
    ass->labelArr.labels = tempLabel;

    // Initialize error array.
    AssemblerError err = ASSEMBLER_NO_ERROR;
    err = AssErrorInit(&(ass->errorArray));
    if (err != ASSEMBLER_NO_ERROR)
    {
        fprintf(stdout, "Assembler errors initialization error.\n");

        free(tempLabel);
        free(temp);
        fclose(ass->outputFile);
        textDtor(&ass->inputText);
        return err;
    }

    DUMP_PRINT("assInit success\n");

    return ASSEMBLER_NO_ERROR;
}


AssemblerError assDtor(Assembler* ass)
{
    assert(ass);
    assert(ass->outputBuffer);

    fclose(ass->outputFile);
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