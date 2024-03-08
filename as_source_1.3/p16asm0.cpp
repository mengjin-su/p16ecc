#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "asm.h"
}
#include "p16asm.h"

EXP_TYPE P16E_asm :: parseItem(item_t *ip, char *buf, int *val, int instruction)
{
	if ( ip )
	{
		char buf0[1024], buf1[1024];
		int  val0, val1;
		EXP_TYPE t0, t1;
		
		if ( instruction == BRA )
		{
			switch ( ip->type )
			{
				case TYPE_SYMBOL:
					if ( buf )	sprintf(buf, "(%s-1-.)", ip->data.str);
					return STR_EXP;
				
				case '.':
					*val = -1;	
					return VAL_EXP;
					
				case '+':
				case '-':
					t1 = parseItem(ip->right,  buf1, &val1, 0);
					if ( ip->left->type == '.' && t1 == VAL_EXP )
					{
						*val = (ip->type == '+')? val1 - 1: -(val1 + 1);
						if ( *val <= 255 && *val >= -256 )
							return VAL_EXP;
					}
				
				default:
					printf("invalid 'bra' operand!\n");
					return BAD_EXP;
			}
		}
		
		switch ( ip->type )
		{
			case TYPE_VALUE:
				*val = ip->data.val;
				return VAL_EXP;

			case TYPE_STRING:
				if ( buf )	sprintf(buf, "\"%s\"", ip->data.str);
				return STR_EXP;
				
			case TYPE_SYMBOL:
				if ( buf )	strcpy(buf, ip->data.str);
				return STR_EXP;

			case UMINUS:
			case INVERSE:			
				t0 = parseItem(ip->left,  buf0, &val0, instruction);
				if ( t0 == VAL_EXP )
					*val = (ip->type == UMINUS)? -val0: ~val0;
				else if ( buf && t0 == STR_EXP )
				{	
					if ( ip->type == UMINUS )
						sprintf(buf, "(0-%s)", buf0);
					else
						sprintf(buf, "(~%s)",  buf0);
				}	
				return t0;
				
			case '.':
				if ( buf ) strcpy(buf, ".");
				return STR_EXP;

			case '+':	case '-':	case '*':	case '/':	case '%':
			case '&':	case '|':	case '^':	case LSHIFT: case RSHIFT:
				t0 = parseItem(ip->left,  buf0, &val0, instruction);
				t1 = parseItem(ip->right, buf1, &val1, instruction);
				if ( t0 == BAD_EXP || t1 == BAD_EXP )
					break;

				if ( t0 == VAL_EXP && t1 == VAL_EXP )
				{
					switch ( ip->type )
					{
						case '+':		*val = val0 + val1; break;
						case '-':		*val = val0 - val1; break;
						case '*':		*val = val0 * val1; break;
						case '/':		*val = val0 / val1; break;
						case '%':		*val = val0 % val1; break;
						case '&':		*val = val0 & val1; break;
						case '|':		*val = val0 | val1; break;
						case '^':		*val = val0 ^ val1; break;
						case LSHIFT:	*val = val0 << val1; break;
						case RSHIFT:	*val = val0 >> val1; break;
					}
					return VAL_EXP;
				}

				if ( t0 == VAL_EXP )
				{
					switch ( ip->type )
					{
						case FSR_PRE_INC:	*val = ((val0&1)<<2)+0; return VAL_EXP;
						case FSR_PRE_DEC:	*val = ((val0&1)<<2)+1; return VAL_EXP;
						case FSR_POST_INC:	*val = ((val0&1)<<2)+2; return VAL_EXP;
						case FSR_POST_DEC:	*val = ((val0&1)<<2)+3; return VAL_EXP;
					}
				}

				if ( t0 == STR_EXP && t1 == VAL_EXP && val1 == 0 )
				{
					if ( buf && (ip->type == '+'    || ip->type == '-' ||
								 ip->type == '|'    || ip->type == '^' ||
								 ip->type == LSHIFT || ip->type == RSHIFT) )
					{
						strcpy(buf, buf0);
						return STR_EXP;
					}
				}

				if ( buf )
				{
					if ( t0 == STR_EXP ) sprintf(buf, "(%s", buf0);
					else				 sprintf(buf, "(%d", val0);
					switch ( ip->type )
					{
						case '+':		strcat(buf, "+");	break;
						case '-':		strcat(buf, "-");	break;
						case '*':		strcat(buf, "*");	break;
						case '/':		strcat(buf, "/");	break;
						case '%':		strcat(buf, "%");	break;
						case '&':		strcat(buf, "&");	break;
						case '|':		strcat(buf, "|");	break;
						case '^':		strcat(buf, "^");	break;
						case LSHIFT:	strcat(buf, "<<");	break;
						case RSHIFT:	strcat(buf, ">>");	break;
					}
					int len = strlen(buf);
					if ( t1 == STR_EXP ) sprintf(&buf[len], "%s)", buf1);
					else				 sprintf(&buf[len], "%d)", val1);
				}
				return STR_EXP;
				
			case FSR_OFFSET:
				t0 = parseItem(ip->left,  buf0, &val0, instruction);
				t1 = parseItem(ip->right, buf1, &val1, instruction);
				if ( t0 == VAL_EXP )
					*val = val0;
				return t0;	
		}
		printf("??? %d\n", ip->type);
	}
	
//	printf("??? %d: %d:%d\n", ip->type, t0, t1);
	return BAD_EXP;
}