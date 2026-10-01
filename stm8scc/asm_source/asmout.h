#ifndef _ASMOUT_H
#define _ASMOUT_H

#include <stdio.h>
extern "C" {
#include "common.h"
}

#include "st8inst.h"

typedef struct {
	bool const_val;	// is in constant value
	char type;
	union {
		unsigned v;
		char    *s;
	};
} value_item;

class asm_obj {

	public:
		int cnt;
		value_item items[5];

	public:
		asm_obj(const ST8_inst_t *);
		asm_obj(int value, char t);
       ~asm_obj();

		void add(int, char);
		void add(char *, char);
		void print(char *buf, int index);
};


class Cout_obj {

	private:
		FILE *fout;
		int  cur_col;
		char cur_type;

	public:
		Cout_obj () { set_output (NULL); }
		void set_output (FILE *fd) { fout = fd; cur_col = 0; }

		void output (asm_obj *op);

		void output (char type, int value);
		void output (char type, char *str);
		void output (char *str);
		void output (int  val);
		void output (char type, item_t *ip);

		// some codes should have both operands in the same line (BSEL, PSEL)
		void output (char type, char *str1, char *str2);
		void output (char type, char *str1, int   val2);
		void output (char type, int   val1, char *str2);
		void output (char type, int   val1, int   val2);

		void flush ();
};

class Cout_lst {

	private:
		FILE *fout;
		int  cur_col;
		char cur_type;
		line_t *cur_line;
		line_t *prt_line;

	public:
		Cout_lst () { set_output (NULL); }
		void set_output (FILE *fd) {
			fout = fd;
			cur_line = NULL;
			prt_line = NULL;
			cur_col = 0;
		}
		void flush ();
		void output(line_t *line);
		void output(int addr, line_t *line);
		void output(int word, line_t *line, int type);
		void output(int word, line_t *line, char type, char ref);
		void output(int wd1, int wd2, line_t *line, char type, char ref);
		void output(char *data, line_t *line);
		void output(int addr, char *buf, line_t *line);

		void output(line_t *line, int addr, int value, int bytes);
		void output(int value, int bytes);
		void output(char *str, int bytes);
		void output(line_t *line, int addr, int code, char *str, int bytes);
};

#endif
