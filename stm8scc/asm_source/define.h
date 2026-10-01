#ifndef _DEFINE_H
#define _DEFINE_H

#include <stdio.h>
#include "common.h"

// --- assembling options ---
#define CASE_SENSITIVE  	0x0001
extern int option_flags;

enum {CODE_SPACE, DATA_SPACE};
extern int code_space;
extern unsigned int opcode, code_ADDR, data_ADDR, eeprom_ADDR;
extern unsigned int line_cnt, *prog_ADDR;
extern line_t *line_ptr;


extern FILE *yyin;
extern char __yyline[];
extern int  error_cnt;

/* ---------------------------------------------------- */
int STR_MATCHn (char *s1, char *s2, int len);
int STR_MATCHi (char *s1, char *s2);

void clr_line_buf (void);

#endif
