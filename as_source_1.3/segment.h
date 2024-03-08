#ifndef _SEGMENT_H
#define _SEGMENT_H

extern "C" {
#include "common.h"
#include "asm.h"
}

enum {
	CODE_SEGMENT = 1,
	DATA_SEGMENT,
	FUSE_SEGMENT,
	CONST_SEGMENT
};

#define isABS(lp)	(lp->attr && lp->attr->isABS)
#define isREL(lp)	(lp->attr && lp->attr->isREL)
#define isBEG(lp)	(lp->attr && lp->attr->isBEG)
#define isEND(lp)	(lp->attr && lp->attr->isEND)

#endif
