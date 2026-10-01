#include <ctype.h>

char isdigit(unsigned char c)
{
    if ( c >= '0' && c <= '9' )
        return 1;

    return 0;
}

char toupper(char c)
{
    if ( c > ('a'-1) && c <= 'z' )
        c &= ~0x20;
    return c;
}