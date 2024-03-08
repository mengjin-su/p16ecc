#ifndef TIMER1_H
#define TIMER1_H

extern unsigned char tmr0Count;

void TMR0_init(void);
void TMR0_delayMs(unsigned char ms);

#endif