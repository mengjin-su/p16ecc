#ifndef TOOL_H
#define TOOL_H

void memset(char *p, char c, int size);
void memcat(char *s1, char *s2);
void memcpy(char *p1, char *p2, unsigned char length);
void strcpy(char *d, char *s);
int  strlen(char *p);
void printByte(char *buf, unsigned char c);
void printHex1(char *buf, unsigned char c);
void printHex2(char *buf, unsigned char c);

#endif