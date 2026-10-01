#ifndef _EXP_H
#define _EXP_H

extern "C" {
#include "define.h"
#include "common.h"
#include "asm.h"
}

typedef enum {
	BAD_VALUE,		// unknown value
	ABS_VALUE,		// absolute value
	LOC_VALUE,		// local (within segment) value
	REL_VALUE,		// relative operand '.'
	OFS_VALUE,		// offset value
	TBD_VALUE		// value will be defined at linking time
} EXP_STATUS;

EXP_STATUS expValue (Cseg *seg, line_t *line, item_t *item, int *value, int cur_addr);
char *expStr  (Cseg *seg, line_t *line, item_t *item);

#endif
