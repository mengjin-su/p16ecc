#ifndef _STRING_H
#define _STRING_H

char strcmp(char *s1, char *s2);
char *strstr(char *s1, char *s2);
char *strchr(char *s, char c);
int  strlen(char *s);
char strcasecmp(char *s1, char *s2);

char memcmp(char *s1, char *s2, int len);
void memset(char *s, char c, int len);
char toupper(char c);

#endif
