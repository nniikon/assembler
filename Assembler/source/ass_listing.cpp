#include "../include/ass_listing.h"

void putCmdToListing(Assembler* ass, AssCommand* cmd, size_t cmdNameBufferPos)
{
    if (ass->listingFile == NULL)
        return;
    assert(cmd);

    #define log(...) fprintf(ass->listingFile, __VA_ARGS__)
    const int BYTES_LENGTH = 18;
    const int NUM_LENGTH = 3;
    const int MAX_SOURCE_CODE_LENGTH = 80;
    const int N_SPACES_IN_TAB = 3;

    size_t nImms = cmd->nLabels + cmd->nNums;
    size_t nTabs = nChrInLine(ass->inputText.line[cmd->line].str, '\t');

    log("%5zu | ", cmd->line);
    size_t shift = 0;
    if (cmd->nRegs > 0)    shift += sizeof(uint8_t);
    if (nImms > 0)         shift += sizeof(int);
    if (cmd->opcode != 0)  shift += sizeof(uint8_t);

    for (size_t i = cmdNameBufferPos; i < cmdNameBufferPos + shift; i++)
    {
        log("%02X", ass->outputBuffer[i]);
        log(" ");
    }
    if (BYTES_LENGTH != (int)shift * NUM_LENGTH)
        log("%*c", BYTES_LENGTH - (int)shift * NUM_LENGTH, ' ');
    log("|\t");

    log("%-*s | ", MAX_SOURCE_CODE_LENGTH - (int) nTabs * N_SPACES_IN_TAB,
                    ass->inputText.line[cmd->line].str);

    if (cmd->opcode != 0) log("%s ", COMMANDS[cmd->cmdID].name);
    if (cmd->nMems > 0)  log("[");
    if (cmd->nRegs > 0)   log("%s ", REGS[cmd->reg].name);
    if (nImms)            log("%d", cmd->imm);
    if (cmd->nMems > 0)  log("]");

    log("\n");

    #undef log
}