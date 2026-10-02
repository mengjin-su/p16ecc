typedef struct {
    int n;
    int m;
    long v, u;
} DATA;

typedef unsigned char BYTE;
typedef unsigned long ULONG;

typedef struct {
	BYTE  	t[3+4];
} TimeStamp3;

typedef struct {
	char type;
	union {
		BYTE bytes[13];		// rx
		struct {			// tx
			ULONG id;
			BYTE  dlc;
			BYTE  buff[8];
		} tx;
	} data;
	TimeStamp3 timeStamp;
} CanRxMsg;

volatile static DATA *ddp;
TimeStamp3 timeStamp;

const CanRxMsg *cmsg = {0};

foo(int *p, DATA *dp)
{
    unsigned char *cp, c;
    CanRxMsg *canp;
/*
    *p = 200;
    *p = 1000;
    p[500] = 1000;
    p[30] = 1000;

    dp->m = 1000;
    dp->n = 100;
    dp->n = (dp->m << 1)*5;

    c = cp[2];
    c = cp[3];
    cp++;
    c = cp[3];
    c = cp[1];
    p[2] >>= 3;

    ddp->u = 10000;
    ddp->v = 20000;
*/
//  canp->timeStamp = timeStamp;
    timeStamp = cmsg->timeStamp;
    //  canp->type = 10;
    *(int*)c = 100;
}
