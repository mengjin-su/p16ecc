#ifndef _DISPLAY_H
#define _DISPLAY_H

void display(node *np, int level, FILE *file = NULL);
void display(char const *str, node *np, int level, FILE *file = NULL);
void dispSrc(node *np, int level, FILE *file = NULL);

void display(attrib *ap, int level, char endln, FILE *file = NULL);
void display(attrib *ap, FILE *file = NULL);
void display(attrib *ap, char ln, FILE *file = NULL);

#endif
