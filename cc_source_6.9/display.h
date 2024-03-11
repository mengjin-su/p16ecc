#ifndef _DEBUG_H
#define _DEBUG_H

#include "pnode.h"

void display (node *np, int level, FILE *file = NULL);
void display (char const *str, node *np, int level, FILE *file = NULL);
void dispSrc (node *np, int level, FILE *file = NULL);

void display (attrib *ap, int level, char endln, FILE *file = NULL);
void display (attrib *ap, FILE *file = NULL);
void display (attrib *ap, char ln, FILE *file = NULL);

void display (Pnode *pcode, FILE *file = NULL);
void display1(Pnode *pcode, FILE *file = NULL);
void display (Item *ip, FILE *file = NULL);

#endif
