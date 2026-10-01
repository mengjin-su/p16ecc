#ifndef KEY_H
#define KEY_H

enum {KEY1=1, KEY2, KEY3};

#define MODE_KEY	KEY1
#define INC_KEY		KEY2
#define DEC_KEY		KEY3

void KEY_init(void);
void KEY_scan(void);
char KEY_read(void);

#endif