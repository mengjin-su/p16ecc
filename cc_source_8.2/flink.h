#ifndef _FLINK_H
#define _FLINK_H

#include "common.h"
#include "dlink.h"

typedef enum {STATIC_DATA, GENERIC_DATA} FDATA_t;

#define ISR_FUNC(fp)		(fp->attr && fp->attr->type == INTERRUPT)
#define STATIC_FUNC(fp)		(fp->attr && fp->attr->isStatic)

class Fnode {
	public:
		char  	 *name;
		attrib 	 *attr;
		Dlink  	 *dlink;
		bool   	  elipsis;
		int    	  dIndex;
		int    	  endLbl;
		NameList *fcall;	// collect func calls.
		Fnode 	 *next;

	public:
		Fnode (char *_name);
		~Fnode();

		int 	parCount(void) { return dlink->dataCount(); }
		int 	parSize(void);		// parameter total size
		attrib *parAttr(int index);	// parameter attrib.

		Dnode	*getData(FDATA_t type, int *index, int *offset);
		Dnode	*getData(Dnode *dnp, FDATA_t type, int *index, int *offset);
		Dnode	*getData(Dlink *dlp, FDATA_t type, int *index, int *offset);
};

class Flink {
	public:
		Fnode *flist;

	public:
		Flink() { flist = NULL; }
		~Flink();

		Fnode *search(char *name);
		void  add(Fnode *fp);
};

#endif
