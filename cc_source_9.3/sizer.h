#ifndef _SIZER_H
#define _SIZER_H
#include "common.h"

typedef enum {
	TOTAL_SIZE = 1, ATTR_SIZE, INDIR_SIZE, SUBDIM_SIZE
} SIZER_OPTION;

class Sizer {
	public:
		int		size(attrib *ap, SIZER_OPTION opt);
		int		size(bool neg_val, int val);
		int 	memberCount(attrib *ap);
		attrib *memberAttr(attrib *ap, int index);
		attrib *memberAttr(attrib *ap, char *id_name);
		attrib *memberAttrClone(attrib *ap, int index);
		attrib *memberAttrClone(attrib *ap, char *id_name);
		int     memberOffset(attrib *ap, char *id_name);
	private:
		int 	size(node *np, int type);
};

extern Sizer sizer;

#endif
