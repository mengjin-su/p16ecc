#ifndef TERM_H
#define TERM_H

extern unsigned char AppFlags;

#define TSTAMP_FLAG		(1 << 0)
#define LF_FLAG			(1 << 1)
#define LF_ENABLED		(AppFlags & LF_FLAG)
#define TSTAMP_ENABLED	(AppFlags & TSTAMP_FLAG)


#define IO_QUEUE_LENGTH		16
extern _linear_ RegIOMsg regIOMsg[IO_QUEUE_LENGTH];
extern BYTE ioTail, ioHead;

void TERM_init(void);
void TERM_input(char *input, unsigned char length);

void TERM_output(void);

#endif	// end of TERM_H