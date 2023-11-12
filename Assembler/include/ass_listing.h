#ifndef ASS_LISTING_H
#define ASS_LISTING_H

#include "ass.h"
#include "stdlib.h"
#include "stdio.h"

struct Assembler;
struct AssCommand;

void putCmdToListing(Assembler* ass, AssCommand* cmd, size_t cmdNameBufferPos);

#endif