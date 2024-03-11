#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "ascii.h"

char *parseRawStr(char *str)
{
	int length = strlen(str);
	char *s = (char*)MALLOC(length+1);
	int i = 0, k;
	int backslash_on = 0;
	while ( *str )
	{
		char c = *str++;
		if ( backslash_on )
		{
			switch ( c )
			{
				case 'n':	s[i++] = '\n';	break;
				case 'a':	s[i++] = '\a';	break;
				case 'r':	s[i++] = '\r';	break;
				case 't':	s[i++] = '\t';	break;
				case 'b':	s[i++] = '\b';	break;
				case 'v':	s[i++] = '\v';	break;
				case 'f':	s[i++] = '\f';	break;

				case '0':	case '1':	case '2':	case '3':
				case '4':	case '5':	case '6':	case '7':
					for (k = 0; k < 3; k++, str++)
					{
						s[i] = ((unsigned char)s[i] << 3) | (c - '0');
						c = *str;
						if ( !(c >= '0' && c <= '7' && k < 2) )	break;
					}
					i++;
					break;

				case 'x':
					for (k = 0; isxdigit(*str); k++, str++)
					{
						c = toupper(*str);
						if ( c > '9' ) c -= 7;
						s[i] = ((unsigned char)s[i] << 4) | (c - '0');
					}
					if ( k != 0 ) i++;
					break;

				default:
					s[i++] = c;
					break;
			}
			backslash_on = 0;
		}
		else if ( c == '\\' )
			backslash_on = 1;
		else
			s[i++] = c;
	}
	return s;
}

int parseRawChr(char *str)
{
	int n;
	switch ( *str )
	{
		case 'n':	return '\n';
		case 'a':	return '\a';
		case 'r':	return '\r';
		case 't':	return '\t';
		case 'b':	return '\b';
		case 'v':	return '\v';
		case 'f':	return '\t';

		case '0':	case '1':	case '2':	case '3':
		case '4':	case '5':	case '6':	case '7':
			n = 0;
			for (;;)
			{
				char c = *str++;
				if ( !(c >= '0' && c <= '7') ) break;
				n = (n << 3) | (c - '0');
			}
			return (n & 0xff);

		case 'x':
			n = 0;
			for (str++; isxdigit(*str); str++)
			{
				char c = toupper(*str);
				if ( c > '9' ) c -= 7;
				n = (n << 4) | (c - '0');
			}
			return (n & 0xff);

		default:
			return (int)*str;
	}
}