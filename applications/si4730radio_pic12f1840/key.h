#ifndef KEY_H
#define KEY_H

#define KEY1_IN(r)	!(r & (1 << 5))		// PA5 - KEY1
#define KEY2_IN(r)	!(r & (1 << 4))		// PA4 - KEY2
#define KEY3_IN(r)	!(r & (1 << 0))		// PA0 - KEY3
#define KEY_MASK	((1 << 0) | (1 << 4) | (1 << 5))
#define KEY_READ	(PORTA & KEY_MASK)
#define ANY_KEY(k)	(k != KEY_MASK)

enum {KEY1=1, KEY2, KEY3, FAST_INC, FAST_DEC};

#define MODE_KEY		KEY1
#define INC_KEY			KEY2
#define DEC_KEY			KEY3

#define MODE_KEY_HOLD   ((PORTA & (1 << 5)) == 0)

void KEY_init(void);
void KEY_scan(void);
char KEY_read(void);

#endif