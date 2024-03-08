#ifndef _RADIO_H
#define _RADIO_H

enum {FM_MODE, AM_MODE};

extern char RADIO_mode;
extern unsigned int RADIO_frequency;

void RADIO_init(void);
void startFM(void);
void startAM(void);
void RADIO_poll(void);

#endif