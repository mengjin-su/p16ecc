#include <pic16e.h>
#include <string.h>

char strcmp(char *s1, char *s2)
{
    for (;; s1++, s2++)
    {
        char c1 = *s1;
        char c2 = *s2;
        char c  = c1 - c2;
        if ( c ) return c;
        if ( !c1 && !c2 ) return 0;
    }
}

int strlen(char *s)
{
    int n = 0;
    while ( *s ) n++, s++;
    return n;
}

char *strchr(char *s, char c)
{
    for (;;s++)
    {
        char ch = *s;
        if ( ch == c ) return s;
        if ( ch == 0 ) return NULL;
    }
}

char strcasecmp(char *s1, char *s2)
{
    for (;; s1++, s2++)
    {
        char c1 = *s1 & ~0x20;
        char c2 = *s2 & ~0x20;
        char n  = c1 - c2;
        if ( n ) return n;
        if ( !c1 && !c2 ) return 0;
    }
}

char *strstr(char *s1, char *s2)
{
    int len = strlen(s2);
    for (; *s1; s1++)
    {
        if ( memcmp(s1, s2, len) == 0 )
            return s1;
    }
    return 0;
}

char memcmp(char *s1, char *s2, int len)
{
    while ( len-- )
    {
        char c1 = *s1++;
        char c2 = *s2++;
        char n = c1 - c2;
        if ( n ) return n;
    }
    return 0;
}

void memset(char *s, char c, int len)
{
    while ( len-- )
        *s++ = c;
}

char toupper(char c)
{
    if ( c >= 'a' && c <= 'z' ) c -= 'a' - 'A';
    return c;
}
