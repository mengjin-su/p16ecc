#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
extern "C" {
#include "define.h"
#include "common.h"
#include "asm.h"
}
#include "symbol.h"
#include "parse.h"
#include "seg.h"
#include "st8asm.h"
#include "st8inst.h"

char *device = NULL;
char extended_inst = 0;
int  stack_end = -1;

//////////////////////////////////////////////////////////////////////////////
int parse(line_t *lptr)
{
    int st8_dev = 0;

    del_symbol_list();     // clean up the symbol list
    clearupSegments();     // clean up the segment list

    while ( lptr )
    {
        line_t *next = lptr->next;
        lptr->next = NULL;

        switch ( lptr->inst )
        {
            case DEVICE:
                if ( itemCount(lptr) > 0 )
                {
                    if ( lptr->items->type == TYPE_SYMBOL ||
                         lptr->items->type == TYPE_STRING  )
                    {
                        device = lptr->items->data.str;
                        if ( STR_MATCHn(device, (char*)"STM8S", 5) )
                           	st8_dev = STM8;
                    }

					item_t *ip;
					for (int i = 1; (ip = itemPtr(lptr, i)) != NULL; i++)
					{

						switch ( ip->type )
						{/**
							case TYPE_SYMBOL:
                                if ( STR_MATCHi (lptr->items->next->data.str, "X") )
		                            extended_inst = 'X';
                                else if ( STR_MATCHi (lptr->items->next->data.str, "R") )
		                        	extended_inst = 'R';
		                        break;
						 **/
		                    case TYPE_VALUE:
		                    	stack_end = ip->data.val & 0xffff;
		                    	break;
						}
                    }
                }
                break;

            case SEGMENT:   // make a new segment
                new Cseg;
                break;

            case EQU:
                if ( lptr->label != NULL && itemStrCheck(lptr->items, lptr->label) )
                {
                    my_yyerror(lptr, "recursive label definition!");
                    return 1;
                }
                break;

            case END:       // end of source code
                freeLines(next);
                next = NULL;
                break;
        }

        if ( curSeg == NULL )
            new Cseg;

        curSeg->addLine(lptr);

        if ( lptr->inst == SEGMENT && curSeg->isABS() )
        {
            if ( lptr->area->item == NULL )
            {
                my_yyerror(lptr, "ABS segment missing address descriptor!");
                return 1;
            }
        }

        lptr = next;
    }

    switch ( st8_dev )
    {
        case 0:
		case STM8:
			// PASS1 - log macros (EQU)
		    for (curSeg = segList; curSeg != NULL; curSeg = curSeg->next)
		        ST8_asm(curSeg, ASM_PASS1);

		    if ( error_cnt > 0 )
		        return 1;

			// PASS2 - log all labels
		    for (curSeg = segList; curSeg != NULL; curSeg = curSeg->next)
		        ST8_asm(curSeg, ASM_PASS2);

		    if ( error_cnt > 0 )
		        return 1;

			// PASS3 - generate outputs
		    for (curSeg = segList; curSeg != NULL; curSeg = curSeg->next)
		        ST8_asm(curSeg, ASM_PASS3);
		    break;

		default:
	        yyerror("unknown device descriptor!");
	}

    return error_cnt;
}
