#include <stdio.h>
#include <string.h>
#include "common.h"
extern "C" {
#include "cc.h"
}
#include "dlink.h"
#include "flink.h"
#include "pnode.h"
#include "display.h"

static bool verbal_asm = false;
static void headMark(int level, FILE *file);
static char *oprName(int token);


void display(node *np, int level, FILE *file)
{
    if ( np == NULL ) return;
	if ( file == NULL ) file = stdout;

	dispSrc(np, level, file);
    switch ( np->type )
    {
        case NODE_CON:
            headMark(level, file);
            fprintf(file, "constant node: %ld ", np->con.value);
            display(np->con.attr, verbal_asm? 0: '\n', file);
            break;

        case NODE_ID:
            headMark(level, file);
            fprintf(file, "id node: %s ", np->id.name);
            display(np->id.attr, verbal_asm? 0: '\n', file);
			display(np->id.dim, level+1, file);
			if ( np->id.fp_decl && np->id.parp )
			{
				headMark(level+1, file);
                fprintf(file, "fptr parameters:");
                if ( !verbal_asm ) fprintf(file, "\n");
				display(np->id.parp, level+1, file);
			}
			if ( np->id.init )
			{
				headMark(level+1, file);
				fprintf(file, "init...:");
				if ( !verbal_asm ) fprintf(file, "\n");
				display(np->id.init, level+1, file);
			}
            break;

        case NODE_STR:
            headMark(level, file);
            fprintf(file, "string node: %s", np->str.str);
            if ( !verbal_asm ) fprintf(file, "\n");
            break;

        case NODE_OPR:
            headMark(level, file);
            if ( np->opr.oper < 127 )
                fprintf(file, "opr node {%d}: %c ", np->opr.nops, np->opr.oper);
            else
                fprintf(file, "opr node {%d}: %s ", np->opr.nops, oprName(np->opr.oper));
			display(np->opr.attr, verbal_asm? 0: '\n', file);

            for (int i = 0; i < np->opr.nops; i++)
                display(np->opr.op[i], level+1, file);
            break;

        case NODE_LIST:
            headMark (level, file);
            fprintf (file, "list node {%d}:", np->list.nops);
            if ( !verbal_asm ) fprintf(file, "\n");

            for (int i = 0; i < np->list.nops; i++)
                display(np->list.ptr[i], level+1, file);
            break;
    }
}

static void headMark(int level, FILE *file)
{
	while ( level-- ) fprintf(file, " ");
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
	sprintf (buf, "(oper. token: %d)", token);
	return buf;
}

void display(attrib *ap, int level, char endln, FILE *file)
{
	if ( file == NULL ) file = stdout;
	if ( ap )//&& ap->type )
	{
        headMark(level, file);
		fprintf(file, "{");

		if ( ap->dataBank )
		{
			if ( ap->dataBank == CONST )
                fprintf(file, "C:");
			else if ( ap->dataBank == LINEAR )
                fprintf(file, "L:");
		}

		if ( ap->isNeg )	  fprintf(file, "- ");
		if ( ap->isVolatile ) fprintf(file, "volatile ");
		if ( ap->isTypedef )  fprintf(file, "typedef ");
		if ( ap->isStatic )   fprintf(file, "static ");
		if ( ap->isExtern )   fprintf(file, "extern ");
		if ( ap->isUnsigned ) fprintf(file, "unsigned ");
		if ( ap->typeName )   fprintf(file, "(%s) ", ap->typeName);

		switch ( ap->type )
		{
			case VOID:			fprintf(file, "void");		break;
			case SBIT:  		fprintf(file, "sbit");  	break;
			case CHAR:  		fprintf(file, "char");    	break;
			case INT:   		fprintf(file, "int");     	break;
			case FPTR:  		fprintf(file, "Fptr");    	break;
			case SHORT: 		fprintf(file, "short");   	break;
			case LONG:  		fprintf(file, "long");    	break;
			case STRUCT:		fprintf(file, "STRUCT"); 	break;
			case UNION: 		fprintf(file, "UNION");  	break;
			case TYPEDEF_NAME:	fprintf(file, "TYPE_NAME");	break;
		}

		for(int i = 0; ap->ptrVect && i < ap->ptrVect[0]; i++)
			if ( ap->ptrVect[i+1] == CONST ) fprintf(file, " C*");
			else							 fprintf(file, " *");

		if ( ap->atAddr )
			display((node *)ap->atAddr, level+1, file);

		if ( ap->dimVect )
		{
			for (int i = 0; i < ap->dimVect[0]; i++)
				fprintf(file, "[%d]", ap->dimVect[i+1]);
		}

		fprintf(file, "}");
		if ( endln ) fprintf(file, "%c", endln);

		if ( endln && (ap->type == STRUCT || ap->type == UNION) )
			display((node*)ap->newData, level+1, file);
	}
}

void display(attrib *ap, char ln, FILE *file)
{
	if ( file == NULL ) file = stdout;
	if ( ap ) display(ap, 0, 0, file);
	if ( ln ) fprintf (file, "%c", ln);
}

void display(attrib *ap, FILE *file)
{
	display(ap, 0, file);
}

void dispSrc(node *np, int level, FILE *file)
{
	if ( np->id.src )
	{
		if ( file == NULL ) file = stdout;
		headMark(level, file);
		fprintf(file, "%s #%d: %s\n", np->id.src->fileName, np->id.src->lineNum, np->id.src->srcCode);
	}
}

////////////// -- display routines for P-code ////////////////
void display(Pnode *pcode, FILE *file)
{
	for(; pcode; pcode = pcode->next)
		display1(pcode, file);
}

void display1(Pnode *pcode, FILE *file)
{
	Fnode *fptr;
	Dnode *dptr;
	bool  show_items = true;
	if ( file == NULL ) file = stdout;
	else { verbal_asm = true; fprintf(file, ";< "); }	// verbal comment in assembly
    switch ( pcode->type )
    {
		case P_SRC_CODE:
			fprintf(file, "P-CODE: SRC ... ");
			break;

        case P_FUNC_BEG:
			fptr = (Fnode*)pcode->items[0]->val.p;
			fprintf(file, "P-CODE: P_FUNC_BEG: %s : ", fptr->name);
			display(fptr->attr, file);
			break;

		case P_FUNC_END:
			fptr = (Fnode*)pcode->items[0]->val.p;
			fprintf(file, "P-CODE: P_FUNC_END: %s \t", fptr->name);
			break;

		case DATA_DECL:
			fprintf(file, "P-CODE: DATA_DECL: %s ", (char*)pcode->items[0]->val.p);
			dptr = (Dnode*)pcode->items[1]->val.p;
			fprintf(file, "[%d bytes] \t", dptr->size());
			show_items = false;
			break;

		case '=':		  	fprintf(file, "P-CODE: '=' \t");		break;
		case ADD_ASSIGN:  	fprintf(file, "P-CODE: '+=' \t");		break;
		case SUB_ASSIGN:  	fprintf(file, "P-CODE: '-=' \t");		break;
		case MUL_ASSIGN:  	fprintf(file, "P-CODE: '*=' \t");		break;
		case DIV_ASSIGN:  	fprintf(file, "P-CODE: '/=' \t");		break;
		case AND_ASSIGN:  	fprintf(file, "P-CODE: '&=' \t");		break;
		case OR_ASSIGN:   	fprintf(file, "P-CODE: '|=' \t");		break;
		case XOR_ASSIGN:  	fprintf(file, "P-CODE: '^=' \t");		break;
		case MOD_ASSIGN:  	fprintf(file, "P-CODE: '%c=' \t", '%');	break;
		case LEFT_ASSIGN: 	fprintf(file, "P-CODE: '<<=' \t");		break;
		case RIGHT_ASSIGN:	fprintf(file, "P-CODE: '>>=' \t");		break;
		case '+':   	  	fprintf(file, "P-CODE: '+' \t");		break;
		case '-':   	  	fprintf(file, "P-CODE: '-' \t");		break;
		case '*':   	  	fprintf(file, "P-CODE: '*' \t");		break;
		case '/':   	  	fprintf(file, "P-CODE: '/' \t");		break;
		case '&':   	  	fprintf(file, "P-CODE: '&' \t");		break;
		case '|':   	  	fprintf(file, "P-CODE: '|' \t");		break;
		case '^':   	  	fprintf(file, "P-CODE: '^' \t");		break;
		case '%':		  	fprintf(file, "P-CODE: '%c' \t", '%');	break;
		case LEFT_OP:  	  	fprintf(file, "P-CODE: '<<' \t");		break;
		case RIGHT_OP: 		fprintf(file, "P-CODE: '>>' \t");		break;

		case POS_OF: 		fprintf(file, "P-CODE: '*p' \t");		break;
		case ADDR_OF:		fprintf(file, "P-CODE: '&n' \t");		break;

		case LABEL:	 		fprintf(file, "P-CODE: LABEL \t");		break;
		case GOTO:	 		fprintf(file, "P-CODE: GOTO \t");		break;
		case NEG_OF: 		fprintf(file, "P-CODE: NEG \t");		break;
		case INC_OP: 		fprintf(file, "P-CODE: '++' \t");		break;
		case DEC_OP: 		fprintf(file, "P-CODE: '--' \t");		break;
		case P_JZ:	 		fprintf(file, "P-CODE: JP_Z \t");		break;
		case P_JNZ:	 		fprintf(file, "P-CODE: JP_NZ \t");		break;
		case P_JEQ:	 		fprintf(file, "P-CODE: JP == \t");		break;
		case P_JNE:	 		fprintf(file, "P-CODE: JP != \t");		break;
		case P_JGT:	 		fprintf(file, "P-CODE: JP > \t");		break;
		case P_JGE:  		fprintf(file, "P-CODE: JP >= \t");		break;
		case P_JLT:  		fprintf(file, "P-CODE: JP < \t");		break;
		case P_JLE:  		fprintf(file, "P-CODE: JP <= \t");		break;
		case P_CAST: 		fprintf(file, "P-CODE: '=?' \t");		break;
		case CASE: 	 		fprintf(file, "P-CODE: CASE \t");		break;
		case RETURN: 		fprintf(file, "P-CODE: RETURN \t");		break;
		case P_ARG_PASS:	fprintf(file, "P-CODE: pass-arg \t"); 	break;
		case CALL:	 		fprintf(file, "P-CODE: CALL \t");		break;
		case P_JBZ:	 		fprintf(file, "P-CODE: JP BitC \t"); 	break;
		case P_JBNZ: 		fprintf(file, "P-CODE: JP BitS \t"); 	break;
		case P_JZ_INC:		fprintf(file, "P-CODE: JZ ++ \t"); 		break;
		case P_JZ_DEC:		fprintf(file, "P-CODE: JZ -- \t"); 		break;
		case P_JNZ_INC: 	fprintf(file, "P-CODE: JNZ ++ \t"); 	break;
		case P_JNZ_DEC:		fprintf(file, "P-CODE: JNZ -- \t"); 	break;
//		case P_MOV_INC:		fprintf(file, "P-CODE: = ++ \t");		break;
//		case P_MOV_DEC:		fprintf(file, "P-CODE: = -- \t");		break;
		case P_DJNZ:		fprintf(file, "P-CODE: DEC & JNZ \t");	break;
		case P_IJNZ:		fprintf(file, "P-CODE: INC & JNZ \t");	break;

		default:
			if ( pcode->type < 128 )
				fprintf(file, "P-CODE: '%c' \t", (char)pcode->type);
			else
				fprintf(file, "P-CODE: [%d] \t", pcode->type);
			break;
	}

	if ( show_items )
	{
		for (int i = 0; i < 3 && pcode->items[i]; i++)
		{
			if ( i ) fprintf(file, ", ");
			display(pcode->items[i], file);
			display(pcode->items[i]->attr, file);
		}
	}
	fprintf(file, "\n");
	verbal_asm = false;
}

void display(Item *ip, FILE *file)
{
	if ( ip == NULL ) return;

	if ( file == NULL ) file = stdout;
	switch ( ip->type )
	{
		case ID_ITEM:
		case LBL_ITEM:
		case STR_ITEM: 	fprintf(file, "%s", ip->val.s);					break;
		case IMMD_ITEM:	fprintf(file, "#%s", ip->val.s);				break;
		case CON_ITEM:	fprintf(file, "%d", ip->val.i);					break;
		case PID_ITEM:  fprintf(file, "[%s:%d]", ip->val.s, ip->bias);	break;
		case DIR_ITEM:  if ( ip->attr->type == SBIT )
							fprintf(file, "(0x%03x:%d)",
										  ip->val.i >> 3, ip->val.i & 7);
						else
							fprintf(file, "(0x%03x)", ip->val.i); 		break;
		case ACC_ITEM:  fprintf(file, "ACC");                     		break;
		case INDIR_ITEM:fprintf(file, "[%c%d]", '\%', ip->val.i+1);		break;
		case TEMP_ITEM: fprintf(file, "%c%d", '\%', ip->val.i+1);		break;
		default: return;
	}
}
