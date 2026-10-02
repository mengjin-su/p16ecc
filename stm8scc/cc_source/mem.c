typedef struct {
    unsigned char DDR;
    unsigned char ODR;
    unsigned char IDR;
	unsigned char CR1;
	unsigned char CR2;
} PORT;

#define PORTA	(PORT*)0x5000
#define PORTB	(PORT*)0x5005

typedef struct {
    int x, y, *ptr;
    char a;
} DATA2;

typedef struct {
    int n, *const m;
    long v;
    DATA2 d, *dp;
} DATA;


bank0 DATA data, *dp;
#define datum *(DATA*)0x30
#define p (DATA *)0x300

foo1(char l);


foo()
{
    int size, array[10];
    char *cp;
    long n;

    data.m++;
    datum.m++;
    datum.n = *datum.m;
    p->m = 1000;
    p->n = 'A';

    DATA *ptr;
    ptr = p + 10;
    size = sizeof(DATA);
    p[2].m = 0456;

    switch ( dp->v )
    {
        case 10000: dp->v += dp->n++;
            p->d.y = 10000 - 2;
            (*(p->d.ptr))++;
            (*((int*)0x30c))++;
    }

    PORTA->ODR = PORTB->IDR;
    foo1(data.dp->a);
    char b = data.dp->a;
    foo1(*cp++);
    *cp++ = 10;
    n = array[size++ & 7];
}
