#ifndef _PARSE_H
#define _PARSE_H

extern "C" {
#include "define.h"
#include "common.h"
#include "asm.h"
}

enum {
	ASM_PASS1 = 1,
	ASM_PASS2,
	ASM_PASS3
};

extern char *device;
extern char extended_inst;
extern int  stack_end;

int parse (line_t *lptr);

#endif
