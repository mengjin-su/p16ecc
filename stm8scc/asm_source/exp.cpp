#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
extern "C" {
#include "define.h"
#include "common.h"
#include "asm.h"
}

#include "symbol.h"
#include "exp.h"
#include "seg.h"

// static buffers ...
#define STR_BUFFER_COUNT	32
static char strBuffer[STR_BUFFER_COUNT][1024];
static int  strBufferIndex;

#define STRBUF()	strBuffer[strBufferIndex++ & (STR_BUFFER_COUNT - 1)]

static bool constExp(char *str);

////////////////////////////////////////////////////////////////////////////////
EXP_STATUS expValue(Cseg *seg, line_t *line, item_t *item, int *value, int cur_addr)
{
    symbol_t *sp;
    int v1, v2;
    EXP_STATUS ret1, ret2;

    if ( item == NULL || line == NULL || seg == NULL )
        return BAD_VALUE;

	switch ( item->type )
	{
		case '#':
		case TYPE_INDIR:
		case TYPE_INDEX:
		case TYPE_INDIR_INDEX:
			return expValue(seg, line, item->left, value, cur_addr);
	}

	ret1 = expValue(seg, line, item->left,  &v1, cur_addr);
	ret2 = expValue(seg, line, item->right, &v2, cur_addr);

    switch ( item->type )
    {
        case '.':           *value = cur_addr;
        					if ( line->inst == SEGMENT )	// segment address can't be '.'
        						return BAD_VALUE;

							return REL_VALUE;

        case TYPE_VALUE:    *value = item->data.val;
        					return ABS_VALUE;

        case TYPE_SYMBOL:  	sp = search_symbol(line, item->data.str);

            				if ( sp == NULL )
            					return TBD_VALUE;

            				if ( sp->item == NULL )
            				    return TBD_VALUE;

            				ret1 = expValue(seg, line, sp->item, value, cur_addr);

            				if ( sp->attr == EQU )
            					return ret1;

            				if ( sp->isABS )
            					return ABS_VALUE;

            				if ( sp->attr == LABEL )
            				{	// if symbol not located in the same segment ?
								if ( sp->seg->seq_num == seg->seq_num )
									return sp->seg->isABS()? ABS_VALUE: LOC_VALUE;

								return sp->seg->isABS()? ABS_VALUE: TBD_VALUE;
							}

            				return ret1;

        case '+':			*value = v1 + v2;
							if ( ret1 == REL_VALUE && ret2 == ABS_VALUE )
							{
								*value = v2;
								return OFS_VALUE;
							}
							break;

        case '-':           *value = v1 - v2;
							if ( ret1 == REL_VALUE && ret2 == ABS_VALUE )
							{
								*value = -v2;
								return OFS_VALUE;
							}
							break;

        case '*':           *value = v1 * v2;		break;
        case '/':           *value = v1 / v2;		break;
        case '%':           *value = v1 % v2;		break;
        case '&':           *value = v1 & v2;		break;
        case '|':           *value = v1 | v2;		break;
        case '^':           *value = v1 ^ v2;		break;

        case UMINUS:        *value = -v1;
       						return (ret1 == LOC_VALUE)? TBD_VALUE: ret1;

        case INVERSE:       *value = ~v1;
							return (ret1 == LOC_VALUE)? TBD_VALUE: ret1;

        case RSHIFT:        *value = v1 >> v2;		break;
        case LSHIFT:        *value = v1 << v2;		break;
	}

   	if ( ret1 == BAD_VALUE || ret2 == BAD_VALUE )
   		return BAD_VALUE;

   	if ( ret1 == ABS_VALUE && ret2 == ABS_VALUE )
   		return ABS_VALUE;

   	if ( ret1 == TBD_VALUE || ret2 == TBD_VALUE )
   		return TBD_VALUE;

	switch ( item->type )
	{
        case RSHIFT:
        case LSHIFT:		return TBD_VALUE;

        case '+':
        case '-':			return (ret1 == ABS_VALUE)? ret2:
        						   (ret2 == ABS_VALUE)? ret1:
        						   (ret1 == TBD_VALUE)? TBD_VALUE:
        						   (ret2 == TBD_VALUE)? TBD_VALUE:
        						   (ret1 == LOC_VALUE)? LOC_VALUE:
        						   						REL_VALUE;
        case '*':
        case '/':
        case '%':
        case '&':
        case '|':
        case '^':           return TBD_VALUE;

        default:            my_yyerror(line, "undetermined item");
    }

    return BAD_VALUE;
}

////////////////////////////////////////////////////////////////////////////////
char *expStr(Cseg *seg, line_t *line, item_t *item)
{
	char *str, *str1, *str2;
	symbol_t *sp;

    switch ( item->type )
    {
		case TYPE_SYMBOL:	sp = (line == NULL)? NULL: search_symbol(line, item->data.str);
		       				if ( sp == NULL )
		       				{
								str = STRBUF();
								strcpy(str, item->data.str);
							}
							else if ( sp->isABS || sp->attr == EQU )
							{
								str = expStr(seg, line, sp->item);
							}
							else if ( sp->item != NULL && sp->item->type != TYPE_VALUE )
							{
								str = expStr(seg, line, sp->item);
							}
							else
							{
								str = STRBUF();
								strcpy(str, sp->name);
							}
							return str;

        case '.':           str = STRBUF();
        					strcpy(str, ".");
        					return str;

        case TYPE_VALUE:    str = STRBUF();
        					sprintf(str, "%d", item->data.val);
        					return str;

        case UMINUS:
        case INVERSE:       str = expStr(seg, line, item->left);
        					str1 = STRBUF();
        					sprintf(str1, "(%c%s)", (item->type==UMINUS)? '-': '~', str);
        					return str1;

        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '&':
        case '|':
        case '^':
        case RSHIFT:
        case LSHIFT:		break;

		case '#':
		case TYPE_INDIR:
		case TYPE_INDEX:
		case TYPE_INDIR_INDEX:
							return expStr(seg, line, item->left);

		default:			str = STRBUF();
							strcpy(str, "?");
							printf("expStr: item->type = %d\n", item->type);
							return str;
	}

	str1 = expStr(seg, line, item->left);
	str2 = expStr(seg, line, item->right);
	str  = STRBUF();

	switch ( item->type )
	{
        case '+':			sprintf(str, "(%s+%s)", str1, str2);	break;
        case '-':           sprintf(str, "(%s-%s)", str1, str2);	break;
        case '*':           sprintf(str, "(%s*%s)", str1, str2);	break;
        case '/':           sprintf(str, "(%s/%s)", str1, str2);	break;
        case '%':           sprintf(str, "(%s%c%s)", str1, '%', str2);	break;
        case '&':           sprintf(str, "(%s&%s)", str1, str2);	break;
        case '|':           sprintf(str, "(%s|%s)", str1, str2);	break;
        case '^':           sprintf(str, "(%s^%s)", str1, str2);	break;
        case RSHIFT:        sprintf(str, "(%s>>%s)", str1, str2);	break;
        case LSHIFT:        sprintf(str, "(%s<<%s)", str1, str2);	break;
	}

	if ( constExp(str1) && constExp(str2) )
	{
		int v1 = atoi(str1);
		int v2 = atoi(str2);

		switch ( item->type )
		{
	        case '+':		sprintf(str, "%d", v1 + v2);	break;
        	case '-':       sprintf(str, "%d", v1 - v2);	break;
	        case '*':       sprintf(str, "%d", v1 * v2);	break;
	        case '/':       sprintf(str, "%d", v1 / v2);	break;
	        case '%':       sprintf(str, "%d", v1 % v2);	break;
	        case '&':       sprintf(str, "%d", v1 & v2);	break;
	        case '|':       sprintf(str, "%d", v1 | v2);	break;
	        case '^':       sprintf(str, "%d", v1 ^ v2);	break;
	        case RSHIFT:    sprintf(str, "%d", v1 >> v2);	break;
	        case LSHIFT:    sprintf(str, "%d", v1 << v2);	break;
		}
	}
	return str;
}

////////////////////////////////////////////////////////////////////////////////
static bool constExp(char *str)
{
	while ( *str && isdigit(*str) ) str++;
	return (*str)? false: true;
}
