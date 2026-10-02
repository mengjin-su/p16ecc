#include <stdio.h>
#include <string.h>
extern "C" {
#include "common.h"
#include "cc.h"
}
#include "flink.h"
#include "dnode.h"
#include "sizer.h"
#include "item.h"
#include "display.h"

static void headMark (int level);
static char *oprName (int token);


void display (node *np, int level)
{
    if ( np == NULL )
        return;

	dispSrc(np, level);
    switch ( np->type )
    {
        case NODE_CON:
            headMark(level);
			printf("constant node: %ld ", np->con.value);
			display(np->con.attr, '\n');
            break;

        case NODE_ID:
            headMark(level);
            printf("id node: %s ", np->id.name);
			display(np->id.attr, '\n');
			display(np->id.dim, level+1);
			if ( np->id.fp_decl && np->id.parp )
			{
				headMark(level+1);
				printf("fptr parameters:\n");
				display(np->id.parp, level+1);
			}
			if ( np->id.init )
			{
				headMark(level+1);
				printf("init...:\n");
				display(np->id.init, level+1);
			}
            break;

        case NODE_STR:
            headMark (level);
            printf ("string node: %s\n", np->str.str);
            break;

        case NODE_OPR:
            headMark (level);
            if ( np->opr.oper < 127 )
                printf("opr node {%d}: %c ", np->opr.nops, np->opr.oper);
            else
                printf("opr node {%d}: %s ", np->opr.nops, oprName(np->opr.oper));
			display(np->opr.attr, '\n');

            for (int i = 0; i < np->opr.nops; i++)
                display(np->opr.op[i], level+1);
            break;

        case NODE_LIST:
            headMark (level);
            printf ("list node {%d}:\n", np->list.nops);

            for (int i = 0; i < np->list.nops; i++)
                display(np->list.ptr[i], level+1);
            break;
    }
}

static void headMark(int level)
{
	while ( level-- ) printf(" ");
}

static char *oprName(int token)
{
	static char buf[256];
    switch ( token )
    {
		case FUNC_HEAD:		return (char*)"FUNC_HEAD";
		case FUNC_DECL: 	return (char*)"FUNC_DECL";
		case DATA_DECL: 	return (char*)"DATA_DECL";
	}
	sprintf(buf, "(oper. token: %d)", token);
	return buf;
}

void display(attrib *ap, int level, char endln)
{
	if ( ap )//&& ap->type )
	{
	    headMark (level);
		printf ("{");
		if ( ap->dataBank )
		{
			if ( ap->dataBank == CONST )
				printf ("C:");
			else if ( ap->dataBank == BANK0 )
				printf ("B0:");
//			else if ( ap->dataBank == FAR )
//				printf ("FAR:");
		}

		if ( ap->isNeg )	  printf("- ");
		if ( ap->isVolatile ) printf("vola ");
		if ( ap->isTypedef )  printf("tdef ");
		if ( ap->isStatic )   printf("stat ");
		if ( ap->isExtern )   printf("extn ");
		if ( ap->isUnsigned ) printf("unsn ");
//		if ( ap->isFar )      printf("far ");
		if ( ap->typeName )   printf("(%s) ", ap->typeName);

		switch ( ap->type )
		{
			case VOID:	printf("void");		break;
			case CHAR:  printf("char"); 	break;
			case INT:   printf("int");  	break;
			case FPTR:  printf("Fptr"); 	break;
			case SHORT: printf("short");	break;
			case LONG:  printf("long");		break;
			case STRUCT:printf("STRUCT"); 	break;
			case UNION: printf("UNION");  	break;
			case TYPEDEF_NAME: printf("TYPE_NAME"); break;
		}

		for(int i = 0; ap->ptrVect && i < ap->ptrVect[0]; i++)
			if ( ap->ptrVect[i+1] == CONST ) printf(" C*");
			else							 printf(" *");

		if ( ap->atAddr )
			display((node *)ap->atAddr, level+1);

		if ( ap->dimVect )
		{
			for (int i = 0; i < ap->dimVect[0]; i++)
				printf ("[%d]", ap->dimVect[i+1]);
		}

		printf ("}");
		if ( endln ) printf ("%c", endln);

		if ( endln )
		{
			if ( ap->type == STRUCT || ap->type == UNION )
				display((node*)ap->newData, level+1);
		}
	}
}

void display(attrib *ap, char ln)
{
	if ( ap ) display(ap, 0, 0);
	if ( ln ) printf("%c", ln);
}

void display(attrib *ap)
{
	display(ap, 0);
}

void dispSrc(node *np, int level)
{
	if ( np->id.src )
	{
		headMark(level);
		printf ("%s #%d: %s\n", np->id.src->fileName, np->id.src->lineNum, np->id.src->srcCode);
	}
}

void display(Pnode *pcode)
{
    for(; pcode; pcode = pcode->next)
    {
		Fnode *fptr;
		Dnode *dptr;
		bool  show_items = true;

        switch ( pcode->type )
        {
			case P_SRC_CODE:
				printf("P-CODE: SRC ... ");
				break;

            case P_FUNC_BEG:
				fptr = (Fnode*)pcode->items[0]->val.p;
				printf("P-CODE: P_FUNC_BEG: %s : ", fptr->name);
				display(fptr->attr);
                break;

            case P_FUNC_END:
				fptr = (Fnode*)pcode->items[0]->val.p;
				printf("P-CODE: P_FUNC_END: %s \t", fptr->name);
                break;

			case DATA_DECL:
				printf("P-CODE: DATA_DECL: %s ", (char*)pcode->items[0]->val.p);
				dptr = (Dnode*)pcode->items[1]->val.p;
				printf("[%d bytes] \t", dptr->size());
				show_items = false;
				break;

			case '=':		  printf("P-CODE: '=' \t");		break;
			case ADD_ASSIGN:  printf("P-CODE: '+=' \t");	break;
			case SUB_ASSIGN:  printf("P-CODE: '-=' \t");	break;
			case MUL_ASSIGN:  printf("P-CODE: '*=' \t");	break;
			case DIV_ASSIGN:  printf("P-CODE: '/=' \t");	break;
			case AND_ASSIGN:  printf("P-CODE: '&=' \t");	break;
			case OR_ASSIGN:   printf("P-CODE: '|=' \t");	break;
			case XOR_ASSIGN:  printf("P-CODE: '^=' \t");	break;
			case MOD_ASSIGN:  printf("P-CODE: '%c=' \t", '%');	break;
			case LEFT_ASSIGN: printf("P-CODE: '<<=' \t");	break;
			case RIGHT_ASSIGN:printf("P-CODE: '>>=' \t");	break;
			case '+':   	  printf("P-CODE: '+' \t");		break;
			case '-': 		  printf("P-CODE: '-' \t");		break;
			case '*': 		  printf("P-CODE: '*' \t");		break;
			case '/': 		  printf("P-CODE: '/' \t");		break;
			case '&': 		  printf("P-CODE: '&' \t");		break;
			case '|': 		  printf("P-CODE: '|' \t");		break;
			case '^': 		  printf("P-CODE: '^' \t");		break;
			case '%':		  printf("P-CODE: '%c' \t", '%');break;
			case LEFT_OP:	  printf("P-CODE: '<<' \t");	break;
			case RIGHT_OP:	  printf("P-CODE: '>>' \t");	break;

			case POS_OF: 	  printf("P-CODE: '*p' \t");	break;
			case ADDR_OF:	  printf("P-CODE: '&n' \t");	break;

			case LABEL:	 	  printf("P-CODE: LABEL \t");	break;
			case GOTO:		  printf("P-CODE: GOTO \t");	break;
			case NEG_OF:	  printf("P-CODE: NEG \t");		break;
			case P_INC:		  printf("P-CODE: '++' \t");	break;
			case P_DEC:		  printf("P-CODE: '--' \t");	break;
			case P_JZ:		  printf("P-CODE: JP_Z \t");	break;
			case P_JNZ:		  printf("P-CODE: JP_NZ \t");	break;
			case P_JEQ:		  printf("P-CODE: JP == \t");	break;
			case P_JNE:		  printf("P-CODE: JP != \t");	break;
			case P_JGT:		  printf("P-CODE: JP > \t");	break;
			case P_JGE: 	  printf("P-CODE: JP >= \t");	break;
			case P_JLT: 	  printf ("P-CODE: JP < \t");	break;
			case P_JLE: 	  printf ("P-CODE: JP <= \t");	break;
			case P_CAST:	  printf ("P-CODE: '=?' \t");	break;
			case CASE: 		  printf ("P-CODE: CASE \t");	break;
			case RETURN: 	  printf ("P-CODE: RETURN \t");	break;
			case P_ARG_PASS:  printf("P-CODE: pass-arg\t"); break;
			case P_ARG_PASS1: printf("P-CODE: pass-arg1\t"); break;
			case CALL:		  printf ("P-CODE: CALL \t");	break;
			case P_CALL:	  printf ("P-CODE: P-CALL \t");	break;
			case P_JBZ:		  printf ("P-CODE: JP BitC \t"); break;
			case P_JBNZ:	  printf ("P-CODE: JP BitS \t"); break;
			case P_JZ_INC:	  printf("P-CODE: JZ ++ \t"); 	break;
			case P_JZ_DEC: 	  printf("P-CODE: JZ -- \t"); 	break;
			case P_JNZ_INC:   printf("P-CODE: JNZ ++ \t");	break;
			case P_JNZ_DEC:	  printf("P-CODE: JNZ -- \t"); 	break;
			case P_MOV_INC:	  printf("P-CODE: = ++ \t");	break;
			case P_MOV_DEC:	  printf("P-CODE: = -- \t");	break;
			case P_DJNZ:	  printf("P-CODE: DEC & JNZ \t");	break;
			case P_IJNZ:	  printf("P-CODE: INC & JNZ \t");	break;
			case SWITCH:	  printf("P-CODE: SWITCH \t");	break;
			case P_DEC_STACK: printf("P-CODE: DecStk \t");	break;
			case P_INC_STACK: printf("P-CODE: IncStk \t");  break;
			case P_MOV:		  printf("P-CODE: P_MOV \t"); 	break;
			default:
				if ( pcode->type < 128 )
					printf ("P-CODE: '%c' \t", (char)pcode->type);
				else
					printf ("P-CODE: [%d] \t", pcode->type);
				break;
		}

		if ( show_items )
		{
			for (int i = 0; i < 3 && pcode->items[i]; i++)
			{
				if ( i ) printf (", ");
				display(pcode->items[i]);
//				if ( pcode->items[i]->type != CON_ITEM )
					display(pcode->items[i]->attr);
			}
		}
		printf ("\n");
	}
}

void display(Item *ip)
{
	if ( ip )
		switch ( ip->type )
		{
			case LBL_ITEM:  //printf("item type: %d, ", ip->type);
			case ID_ITEM:
			case STR_ITEM: 	printf("%s", ip->val.s);				break;
			case IMMD_ITEM:	printf("#%s", ip->val.s);				break;
			case CON_ITEM:	printf("%d", ip->val.i);				break;
			case PID_ITEM:  printf("[%s:%d]", ip->val.s, ip->bias);	break;
			case DIR_ITEM:  printf("(0x%03x)", ip->val.i); 			break;
			case ACC_ITEM:  printf("ACC");                     		break;
			case INDIR_ITEM:printf("[%c%d]", '\%', ip->val.i+1);	break;
			case TEMP_ITEM: printf("%c%d", '\%', ip->val.i+1);		break;
			default: return;
		}
}
