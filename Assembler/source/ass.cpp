#include "../include/ass.h"

//#define DUMP_DEBUG   
#include "../../lib/dump.h"

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

static CommandError setCommandName(const char** str, Assembler* ass, AssCommand* cmd);

static CommandError setLabelName(const char* str, Assembler* ass);

static bool trySetReg(const char** str, const size_t size, AssCommand* cmd);

static bool trySetLabel(const char** str, const size_t size, Assembler* ass, AssCommand* cmd);

static bool trySetNumber(const char** str, const size_t size, AssCommand* cmd);

static CommandError setArgs(const char** str, Assembler* ass, AssCommand* command);

static void adjustCommandCode(AssCommand* command);

static void putCommandToBuffer(Assembler* ass, AssCommand* cmd);


static AssemblerError putBufferToFile(const Assembler* ass)
{
    size_t refSize = fwrite(ass->outputBuffer, sizeof(uint8_t), ass->outputBufferPos, ass->outputFile);
    if (refSize != ass->outputBufferPos)
        return ASSEMBLER_FWRITE_ERROR;
    return ASSEMBLER_NO_ERROR;
}


static CommandError setCommandName(const char** str, Assembler* ass, AssCommand* cmd)
{
    assert(str);
    assert(ass);

    DUMP_PRINT("Setting command name:\n");
    *str = skipWhiteSpaces(*str);
    size_t size = getWordSize(*str, " ");

    DUMP_PRINT("Command name length: <%zu>, name: <%.*s>\n", size, (int)size, *str);

    for (size_t cmdID = 0; cmdID < AMOUNT_OF_COMMANDS; cmdID++)
    {
        if (strncasecmp(*str, COMMANDS[cmdID].name, size) == 0)
        {
            DUMP_PRINT("Command name: <%s>\n", COMMANDS[cmdID].name);
            DUMP_PRINT("Command code: <%d>\n", COMMANDS[cmdID].code & CMD_COMMAND_BITS);

            cmd->opcode = COMMANDS[cmdID].code & CMD_COMMAND_BITS;
            DUMP_PRINT("COMMAND CODE: %d\n", cmd->opcode);
            cmd->cmdID = cmdID;

            (*str) += size;

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
        fprintf(stderr, "You wrote more than %zu labels and hit the limit.\n", MAX_NUMBER_OF_LABELS);
        fprintf(stderr, "You can change this value in ./Assembler/include/ass_config.h\n.");
        return CMD_HIT_MAX_LABLES;
    }

    if (nChrInLine(str, ':') > 1)
        return CMD_TOO_MANY_COLONS;

    size_t size = getWordSize(str, ":");

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


static bool trySetReg(const char** str, const size_t size, AssCommand* cmd)
{
    assert(str);

    // Optimization.
    if (!isalpha((*str)[0]))
        return false;

    for (size_t i = 0; i < AMOUNT_OF_REGISTERS; i++)
    {
        if (strncasecmp(*str, REGS[i].name, size) == 0)
        {
            DUMP_PRINT("\"%.*s\"\n", (int)size, *str);
            DUMP_PRINT("Register <%s>\n", REGS[i].name);
            DUMP_PRINT("id = <%d>\n", (int) REGS[i].id);

            cmd->reg = REGS[i].id;

            return true;
        }
    }
    return false;
}

static bool trySetLabel(const char** str, const size_t size, Assembler* ass, AssCommand* cmd)
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

            cmd->imm = ass->labelArr.labels[i].adress;

            DUMP_PRINT("Putting label success\n");
            return true;
        }
    }

    // If the label hasn't been met before...
    cmd->nLabels++;
    cmd->imm = LABEL_POISON;
    putCommandToBuffer(ass, cmd);

    DUMP_PRINT("No such label was found\n");

    return false;
}

static inline bool isNumber(char chr) 
{
    return (isdigit(chr) || chr == '+' || chr == '-' || chr == '.');
}

static bool trySetNumber(const char** str, const size_t size, AssCommand* cmd)
{
    DUMP_PRINT("Putting number to buffer.\n");
    for (size_t i = 0; i < size; i++)
    {
        if (!isNumber((*str)[i]))
            return false;
    }
    int num = atoi(*str) * FLOATING_POINT_COEFFICIENT;

    DUMP_PRINT("\"%.*s\"\n", (int)size, *str);
    DUMP_PRINT("number: <%d>\n", num);

    cmd->imm = num;

    return true;
}


static CommandError checkNumberOfArguments(AssCommand* command)
{
    DUMP_PRINT("nMems   = <%zu>\n", command->nMems);
    DUMP_PRINT("nLabels = <%zu>\n", command->nLabels);
    DUMP_PRINT("nNums   = <%zu>\n", command->nNums);
    DUMP_PRINT("nRegs   = <%zu>\n", command->nRegs);

    if (command->nMems > 1)                    return CMD_TOO_MANY_BRACKETS;
    if (command->nLabels > 1)                  return CMD_TOO_MANY_LABELS;
    if (command->nNums > 1)                    return CMD_TOO_MANY_NUMBERS;
    if (command->nRegs > 1)                    return CMD_TOO_MANY_REGISTERS;
    if (command->nLabels + command->nNums > 1) return CMD_TOO_MANY_ARGUMENTS;

    return CMD_NO_ERROR;
}


static CommandError setArgs(const char** str, Assembler* ass, AssCommand* command)
{
    assert(str);
    assert(ass);
    assert(command);

    DUMP_PRINT("setArgs started with <%s>\n", *str);

    CommandError err = checkNumberOfArguments(command);
    if (err != CMD_NO_ERROR)
        return err;

    *str = skipDelims(*str, ARG_DELIMS);
    size_t size = getWordSize(*str, ARG_DELIMS);

    // Check if the recursion is over.
    if ((*str)[0] == '\0') 
        return CMD_NO_ERROR;

    if (trySetReg(str, size, command))
    {
        command->nRegs++;
        *str += size;
        return setArgs(str, ass, command);
    }
    else if (trySetNumber(str, size, command)) 
    {
        command->nNums++;
        *str += size;
        return setArgs(str, ass, command);
    }
    else if ((*str)[0] == '[')
    {
        command->nMems++;
        (*str)++;
        return setArgs(str, ass, command);
    }
    else if ((*str)[0] == ']')
    {
        // TODO: add check
        return CMD_NO_ERROR;
    }
    else if (trySetLabel(str, size, ass, command))
    {
        command->nLabels++;
        *str += size;
        return setArgs(str, ass, command);
    }
    else
    {
        return CMD_INVALID_ARG;
    }

    DUMP_PRINT("setArgs successful\n");
    return CMD_NO_ERROR;
}


static void adjustCommandCode(AssCommand* cmd)
{
    assert(cmd);

    DUMP_PRINT("Started adjusting command code\n");
    DUMP_PRINT("Command code: %d\n", cmd->opcode);
    if (cmd->nLabels == 1)
    {
        cmd->opcode |= CMD_IMMEDIATE_BIT;
        DUMP_PRINT("add imm(label) bit\n");
    }
    if (cmd->nNums == 1)
    {
        cmd->opcode |= CMD_IMMEDIATE_BIT;
        DUMP_PRINT("add imm(number) bit\n");
    }
    if (cmd->nMems == 1)
    {
        cmd->opcode |= CMD_MEMORY_BIT;
        DUMP_PRINT("add ram bit\n");
    }
    if (cmd->nRegs == 1)
    {
        cmd->opcode |= CMD_REGISTER_BIT;
        DUMP_PRINT("add reg bit\n");
    }

    DUMP_PRINT("Command code: %d\n", cmd->opcode);
    DUMP_PRINT("Adjustment success\n");
}


static void putCommandToBuffer(Assembler* ass, AssCommand* cmd)
{
    assert(ass);
    assert(cmd);

    DUMP_PRINT("Started putting command to the buffer.\n");

    // Set the opcode.
    ass->outputBuffer[ass->outputBufferPos] = cmd->opcode;
    ass->outputBufferPos += sizeof(uint8_t);
    DUMP_PRINT("OPCODE: %d\n", cmd->opcode);

    if (cmd->nRegs > 0)
    {
        ass->outputBuffer[ass->outputBufferPos] = cmd->reg;
        ass->outputBufferPos += sizeof(uint8_t);
        DUMP_PRINT("REGS: %d\n", cmd->reg);
    }
    if (cmd->nLabels + cmd->nNums > 0)
    {
        memcpy(ass->outputBuffer + ass->outputBufferPos, &(cmd->imm), sizeof(int));
        ass->outputBufferPos += sizeof(int);
        DUMP_PRINT("IMM: %d\n", cmd->imm);
    }
}


#define PUSH_ON_ERR(err)                                            \
    if ((err) != CMD_NO_ERROR)                                      \
    {                                                               \
        if (isLastAssembly)                                         \
        {                                                           \
            AssError tempError = {err, line, ass->inputFileName};   \
            pushAssErrArray(&(ass->errorArray), &tempError);        \
        }                                                           \
        SET_UP_CONTINUE(hasComments, commentPtr);                   \
    }                                                               \


#define SET_UP_CONTINUE(hasComments, commentPtr)            \
    if (hasComments)                                        \
        *commentPtr = COMMENTS_CHR;                         \
    if (isLastAssembly)                                     \
        putCmdToListing(ass, &command, cmdNameBufferPos);   \
    continue                                                \


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

        AssCommand command = {};
        command.line = line;

        const char* str = ass->inputText.line[line].str;
        size_t cmdNameBufferPos = ass->outputBufferPos;

        // Skip insignificant characters.
        str = skipWhiteSpaces(str);
        if (str[0] == '\0')
        {
            SET_UP_CONTINUE(hasComments, commentPtr);
        }

        // Check the correctness of brackets.
        if (nChrInLine(str, '[') != nChrInLine(str, ']'))
            PUSH_ON_ERR(CMD_MISMATCHED_BRACKET);

        IF_DUMP_DEBUG(fputc('\n', stderr));
        DUMP_PRINT("Analysing line num: %ld\n", line);
        DUMP_PRINT("<%s>\n", str);
        DUMP_PRINT("Current shift: %zu\n", ass->outputBufferPos);

        // Save the command name position.

        CommandError err = CMD_NO_ERROR;

        // If the line has ':' in it...
        if (strchr(str, ':') != NULL) // TODO: fix `la bel:`
        {
            err = setLabelName(str, ass); // Set the label.
            PUSH_ON_ERR(err);
            SET_UP_CONTINUE(hasComments, commentPtr);
        }
        else
        {
            // Treat the first word as a command name.
            err = setCommandName(&str, ass, &command);
            PUSH_ON_ERR(err);
        }
        // Set the args.
        err = setArgs(&str, ass, &command);
        PUSH_ON_ERR(err);

        adjustCommandCode(&command);

        putCommandToBuffer(ass, &command);

        if (isLastAssembly)
            putCmdToListing(ass, &command, cmdNameBufferPos);

        err = checkCommandCorrectness(&command, ass, cmdNameBufferPos);
        PUSH_ON_ERR(err);

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


AssemblerError assInit(Assembler* ass, const AssemblerInitInfo* info)
{
    assert(ass);
    assert(info);

    DUMP_PRINT("assInit started\n");

    // Initialize input Text from the file. 
    FileError error = textInit(info->inputFileName, &ass->inputText);
    if (error != PARSE_NO_ERROR)
    {
        fprintf(stderr, "Initializing text error\n");
        return ASSEMBLER_PARSE_ERROR;
    }

    // Open the output file.
    ass->outputFile = fopen(info->outputFileName, "wb");
    if (ass->outputFile == NULL)
    {
        textDtor(&ass->inputText);
        fprintf(stderr, "failed to open %s\n", info->outputFileName);
        return ASSEMBLER_OPEN_FILE_ERROR;
    }

    // Initialize output Buffer. FIXME: Realloc output buffer if needed.
    uint8_t* temp = (uint8_t*) calloc(ass->inputText.nLines, MAX_NUMBER_LINE_CMD * sizeof(int));
    if (temp == NULL)
    {
        fprintf(stderr, "Allocating memory error.\n");

        fclose(ass->outputFile);
        textDtor(&ass->inputText);

        return ASSEMBLER_ALLOCATION_ERROR;
    }
    ass->outputBuffer = temp;
    ass->outputBufferPos = 0lu;

    ass->inputFileName = info->inputFileName;

    // Initialize LabelArr
    ass->labelArr.emptyLabel = 0;
    ass->labelArr.nLabels    = 0;
    Label* tempLabel = (Label*) calloc(MAX_NUMBER_OF_LABELS, sizeof(Label));
    if (tempLabel == NULL)
    {
        fprintf(stderr, "Allocating memory error.\n");

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
        fprintf(stderr, "Assembler errors initialization error.\n");

        free(tempLabel);
        free(temp);
        fclose(ass->outputFile);
        textDtor(&ass->inputText);
        return err;
    }

    if (info->listingFileName != NULL)
    {
        ass->listingFile = fopen(info->listingFileName, "w");
        if (ass->listingFile == NULL)
        {
            fprintf(stderr, "Failed to open %s.\n", info->listingFileName);

            free(tempLabel);
            free(temp);
            fclose(ass->outputFile);
            textDtor(&ass->inputText);
            return ASSEMBLER_OPEN_FILE_ERROR;
        }
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

    for (int bitPos = (int)(nBits - 1); bitPos >= (int) NUMBER_OF_CMD_BITS; bitPos--)
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