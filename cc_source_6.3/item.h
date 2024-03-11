#ifndef _ITEM_H
#define _ITEM_H

#include "common.h"

////////////////////////////////////////////////////////////////
typedef enum {
    STR_ITEM = 1,	// string Item
    CON_ITEM,		// constant Item
    ID_ITEM,		// id Item
    TEMP_ITEM,		// temp Item
    ACC_ITEM,		// ACC Item
    LBL_ITEM,		// label Item
    IMMD_ITEM,		// immediate Item
    INDIR_ITEM,		// indirect address Item
    DIR_ITEM,		// direct address Item
    PTR_ITEM,		// pointer Item
    PID_ITEM,		// pointer variable Item
} ITEM_TYPE;

typedef union {
    int      i;
    char    *s;
    void    *p;
} value;

/////////////////////////////////////////////////////////////////////////////
class Item {

	public:
	    ITEM_TYPE   type;
	    value       val;
	    int			bias;
	    attrib      *attr;  // attributes
		void     	*home;	// owner of id (ID_ITEM only)
	    Item 		*next;	// linking chain

	public:
		Item (ITEM_TYPE t);
		~Item ();

		Item *clone (void);
		void updateAttr (attrib *ap, int data_bank = -1);
		void updatePtr  (int *p);
		void updateName (char *new_name);
		bool isWritable (void);
		bool isOperable (void);
		bool isMonoVal  (void);
		bool isAccePtr  (void);
		bool isBitVal   (void);
		attrib *acceAttr(void);
		int	 stepSize   (void);
		int  acceSize   (void);
		int  acceSign   (void);
		int  storSize	(void);
};

Item *intItem(int val, attrib *ap=NULL);
Item *strItem(char *str);
Item *lblItem(int lbl);
Item *lblItem(char *str);
Item *idItem (char *name, attrib *attr=NULL);
Item *ptrItem(void *p);
Item *immdItem(char *str, attrib *attr);
Item *tmpItem(int index, attrib *attr=NULL);
Item *accItem(attrib *attr);

bool moveMatch(Item *ip0, Item *ip1);
attrib *maxMonoAttr(Item *ip0, Item *ip1, int op);
attrib *maxSizeAttr(Item *ip0, Item *ip1);
bool comparable(int code, Item *ip0, Item *ip1);
bool same(Item *ip0, Item *ip1, bool fsr_check = false);
bool sameTemp(Item *ip0, Item *ip1);
bool overlap(Item *ip0, Item *ip1);
bool related(Item *ip0, Item *ip1);

#endif