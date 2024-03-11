#ifndef _COMMON_H
#define _COMMON_H

#ifdef __cplusplus
 extern "C" {
#endif

int  _main(char *filename);
void *MALLOC(int size);
char *dupStr(char *s);

#ifdef __cplusplus
 }
#endif

#endif 