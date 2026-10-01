#ifndef _SEG_H
#define _SEG_H

extern "C" {
#include "define.h"
#include "common.h"
#include "asm.h"
}

enum {
	CODE_SEGMENT = 1,
	DATA_SEGMENT,
	EPROM_SEGMENT,
	FUSE_SEGMENT,
	CONST_SEGMENT
};

class Cseg {
	public:
		Cseg  	*next;
		int		seq_num;
		line_t	*lines;
		unsigned int addr;

	public:
		Cseg ();
		~Cseg ();
		int type (void);
		char *name (void);

		bool isABS (void);
		bool isREL (void);
		bool isOVR (void);
		bool isBEG (void);
		bool isEND (void);
		void addLine (line_t *lptr);
};

extern Cseg *segList;
extern Cseg *curSeg;

void clearupSegments (void);

#endif
