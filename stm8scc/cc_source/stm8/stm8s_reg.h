#ifndef _ST8_REG_H
#define _ST8_REG_H

#include "../item.h"
#include "stm8s_asm.h"

#define REG_A	((char*)"%A")
#define REG_X	((char*)"%X")
#define REG_Y	((char*)"%Y")
#define REG_SP	((char*)"%SP")
#define REG_XH	((char*)"%XH")
#define REG_XL	((char*)"%XL")
#define REG_YH	((char*)"%YH")
#define REG_YL	((char*)"%YL")
#define REG_CC	((char*)"%CC")


class STM8S_reg {

	public:
		const char *regName;
		int   regSize;
		int   regVal;
		int   bias;
		bool  loaded;
		bool  numLoad;
		Item *trace;

	public:
		STM8S_reg(const char *name, int size);
		~STM8S_reg();
		void reset(void);
		void inc(int amount);
		void dec(int amount);
		bool load(int n, int offset);

		// for register tracing ...
		bool search(Item *ip, int offset);
		void remove(Item *ip);
		void assign(Item *ip, int offset);
		void add   (Item *ip, int offset);

		bool searchPtr(Item *ip, int offset);
		void removePtr(Item *ip);
		void assignPtr(Item *ip, int offset);
		void addPtr   (Item *ip, int offset);
};
/*
extern STM8S_reg STM8S_ACC;
extern STM8S_reg STM8S_X, STM8S_Y;

//STM8S_reg *reg_p(char *reg);

bool REG_load (char *reg, int n, int offset);
void REG_inc  (char *reg, int amount);
void REG_dec  (char *reg, int amount);
void REG_reset(char *reg);
void REG_zero (char *reg);

class STM8S_PTR {

	private:
		char src;

	public:
		STM8S_PTR() { free(); }
		char *req(void);
		char *req(char *reg16);
		char *use(char *reg16);
		void free() { src = -1; }
		void free(char *ptr);
		void free(char *ptr1, char *ptr2);
		void free(char *ptr1, char *ptr2, char *ptr3);
		int  avail(void);
		int  cost(Item *ip0, bool acce_word);
		int  cost(Item *ip0, Item *ip1, bool acce_word) {
			return cost(ip0, acce_word) + cost(ip1, acce_word);
		}
		int  cost(Item *ip0, Item *ip1, Item *ip2, bool acce_word) {
			return cost(ip0, ip1, acce_word) + cost(ip2, acce_word);
		}
		char *Hi(char *reg16);
		char *Lo(char *reg16);
		void status ();
};

extern STM8S_PTR STM8S_Ptr;

void traceAssign(char *reg, Item *ip, int offset);
void traceAdd   (char *reg, Item *ip, int offset);
bool traceSearch(char *reg, Item *ip, int offset);

void traceReset (char *reg);
void traceZero  (char *reg);
void traceRemove(Item *ip);

void traceAssignPtr(char *reg, Item *ip, int offset);
void traceAddPtr   (char *reg, Item *ip, int offset);
bool traceSearchPtr(char *reg, Item *ip, int offset);
*/

#if 1
class STM8S_IREG {
	private:
		STM8S_ASM *stm8_asm;
		char *name;
		Item *ip;
		int ptrOffset;
		int varOffset;
	public:
		STM8S_IREG(char *name, STM8S_ASM *stm8_asm);
		void reset(void);
		bool load(Item *, int offset=0);
		void inc(int amount, bool asm_output=false);
};
#endif

#endif
