#ifndef _SIZER_H
#define _SIZER_H
#include "common.h"

typedef enum {
	TOTAL_SIZE = 1, ATTR_SIZE, INDIR_SIZE, SUBDIM_SIZE
} SIZER_OPTION;

int		sizer(attrib *ap, SIZER_OPTION opt);
int		sizer(bool neg_val, int val);
int 	memberCount(attrib *ap);
attrib *memberAttr(attrib *ap, int index);
attrib *memberAttr(attrib *ap, char *id_name);
attrib *memberAttrClone(attrib *ap, int index);
attrib *memberAttrClone(attrib *ap, char *id_name);
int     memberOffset(attrib *ap, char *id_name);

#endif
