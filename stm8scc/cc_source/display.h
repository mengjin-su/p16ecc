#ifndef _DEBUG_H
#define _DEBUG_H
extern "C" {
#include "common.h"
}
#include "pnode.h"
#include "pcoder.h"
#include "item.h"

void display (node *np, int level);
void display (char const *str, node *np, int level);
void display (attrib *ap, int level, char endln);
void display (attrib *ap);
void display (attrib *ap, char ln);
void dispSrc (node *np, int level);
void display (Pnode *pcode);
void display (Item *ip);

#endif
