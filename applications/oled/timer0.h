#ifndef TIMER0_H
#define TIMER0_H

extern unsigned char tmr0Count;

void TMR0_init(void);
void TMR0_delayMs(unsigned char ms);

#endif