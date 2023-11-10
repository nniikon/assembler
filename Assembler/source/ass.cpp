#include "../include/ass.h"

#define DUMP_DEBUG   
#include "../../lib/dump.h"


struct AssCommand
{
    size_t nRegs;
    size_t nNums;
    size_t nLabels;
    size_t nMems;

    size_t cmdID;

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

static bool tryPutRegToBuffer(const char** str, const size_t size, Assembler* ass);

static bool tryPutLabelToBuffer(const char** str, const size_t size, Assembler* ass);

static bool tryPutNumberToBuffer(const char** str, const size_t size, Assembler* ass);

static CommandError setArgs(const char** str, Assembler* ass, AssCommand* command);

static void adjustCommandCode(AssCommand* command, Assembler* ass, size_t pos);


static AssemblerError putBufferToFile(const Assembler* ass)
{
    size_t refSize = fwrite(ass->outputBuffer, sizeof(char), ass->outputBufferPos, ass->outputFile);
    if (refSize != ass->outputBufferPos)
        return ASSEMBLER_FWRITE_ERROR;
    return ASSEMBLER_NO_ERROR;
}


static CommandError putCommandNameToBuffer(const char** str, Assembler* ass, AssCommand* cmd)
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
            ass->outputBuffer[ass->outputBufferPos] = (COMMANDS[cmdID].code & CMD_COMMAND_BITS);
            ass->outputBufferPos += sizeof(uint8_t); 

            DUMP_PRINT("Command name: <%s>\n", COMMANDS[cmdID].name);
            DUMP_PRINT("Command code: <%d>\n", COMMANDS[cmdID].code & CMD_COMMAND_BITS);

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
        fprintf(stdout, "You wrote more than %zu labels and hit the limit.\n", MAX_NUMBER_OF_LABELS);
        fprintf(stdout, "You can change this value in ./Assembler/include/ass_config.h\n.");
        return CMD_HIT_MAX_LABLES;
    }

    if (nChrInLine(str, ':') > 1)
        return CMD_TOO_MANY_COLONS;
    
    str = skipWhiteSpaces(str);
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


static bool tryPutRegToBuffer(const char** str, const size_t size, Assembler* ass)
{
    assert(str);
    assert(ass);

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

            ass->outputBuffer[ass->outputBufferPos] = REGS[i].id;
            ass->outputBufferPos += sizeof(uint8_t);

            return true;
        }
    }
    return false;
}


static bool tryPutLabelToBuffer(const char** str, const size_t size, Assembler* ass)
{
    assert(str);
    assert(ass);

    // Optimization.
    if (!isalpha((*str)[0]))
        return false;

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

static bool tryPutNumberToBuffer(const char** str, const size_t size, Assembler* ass)
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

    memcpy(ass->outputBuffer + ass->outputBufferPos, &num, sizeof(int));
    ass->outputBufferPos += sizeof(int);

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

    if (tryPutRegToBuffer(str, size, ass))
    {
        command->nRegs++;
        *str += size;
        return setArgs(str, ass, command);
    }
    else if (tryPutLabelToBuffer(str, size, ass))
    {
        command->nLabels++;
        *str += size;
        return setArgs(str, ass, command);
    }
    else if (tryPutNumberToBuffer(str, size, ass)) 
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

    if (command->nLabels == 1)
    {
        ass->outputBuffer[pos] |= CMD_IMMEDIATE_BIT;
        DUMP_PRINT("add imm(label) bit\n");
    }
    if (command->nNums == 1)
    {
        ass->outputBuffer[pos] |= CMD_IMMEDIATE_BIT;
        DUMP_PRINT("add imm(number) bit\n");
    }
    if (command->nMems == 1)
    {
        ass->outputBuffer[pos] |= CMD_MEMORY_BIT;
        DUMP_PRINT("add ram bit\n");
    }
    if (command->nRegs == 1)
    {
        ass->outputBuffer[pos] |= CMD_REGISTER_BIT;
        DUMP_PRINT("add reg bit\n");
    }

    DUMP_PRINT("Command code: %d\n", ass->outputBuffer[pos]);
    DUMP_PRINT("Adjustment success\n");
}


#define PUSH_AND_CONTINUE_ON_ERR(err)\
    if ((err) != CMD_NO_ERROR)\
    {\
        if (isLastAssembly)\
        {\
            AssError tempError = {err, line, ass->inputFileName};\
            pushAssErrArray(&(ass->errorArray), &tempError);\
        }\
        FIX_COMMENTS_AND_CONTINUE(hasComments, commentPtr);\
    }\


#define FIX_COMMENTS_AND_CONTINUE(hasComments, commentPtr)\
    if (hasComments)\
        *commentPtr = COMMENTS_CHR;\
    continue\


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
        skipWhiteSpaces(str);
        if (str[0] == '\0')
        {
            FIX_COMMENTS_AND_CONTINUE(hasComments, commentPtr);
        }

        // Check the correctness of brackets.
        if (nChrInLine(str, '[') != nChrInLine(str, ']'))
            PUSH_AND_CONTINUE_ON_ERR(CMD_MISMATCHED_BRACKET);

        IF_DUMP_DEBUG(fputc('\n', stderr));
        DUMP_PRINT("Analysing line num: %ld\n", line);
        DUMP_PRINT("<%s>\n", str);

        AssCommand command = {};
        command.line = line;

        // Save the command name position.
        size_t cmdNameBufferPos = ass->outputBufferPos;

        CommandError err = CMD_NO_ERROR;

        // If the line has ':' in it...
        //function:
        if (strchr(str, ':') != NULL)
        {
            err = setLabelName(str, ass); // Set the label.
            PUSH_AND_CONTINUE_ON_ERR(err);
            FIX_COMMENTS_AND_CONTINUE(hasComments, commentPtr);
        }
        else
        {
            // Treat the first word as a command name.
            err = putCommandNameToBuffer(&str, ass, &command);
            PUSH_AND_CONTINUE_ON_ERR(err);
        }
        // Set the args.
        err = setArgs(&str, ass, &command);
        PUSH_AND_CONTINUE_ON_ERR(err);

        adjustCommandCode(&command, ass, cmdNameBufferPos);

        err = checkCommandCorrectness(&command, ass, cmdNameBufferPos);
        PUSH_AND_CONTINUE_ON_ERR(err);

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

    // Initialize output Buffer. FIXME: Realloc output buffer if needed.
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