#ifndef _MEM_H
#define _MEM_H

typedef enum {DATA_MEMORY, CODE_MEMORY} MEMORY_TYPE;

#define GENERAL_MEMORY(a)	((a&0x7f) >= 0x20 && (a&0x7f) < 0x70)
#define TO_LINEAR_ADDR(a)	(0x2000 + (a>>7)*80 + (a&0x7f) - 0x20)
#define TO_BANKED_ADDR(a)	((((a&0x1fff)/80)<<7) + (a&0x1fff)%80+0x20)

#define LINEAR_DATA_FLAG	0x01
#define	PAGE_CHECK_FLAG		0x02

class Memory {
	public:
		Memory(MEMORY_TYPE type);
		~Memory();
		void init(line_t *lp);
		void resize(int size);
		void init(void);
		void reset(void);
		void blank(int start, int length);
		bool getSpace(int start_addr, int req_size, int flag=0);	// fixed address
		bool getSpace(int req_size, int *actual_addr, int flag=0);	// floating address
		int  memUsed(int *unused);
		void printSpace(FILE *);
		void display(int);

	private:
		MEMORY_TYPE type;
		char 		*mem;
		int			maxSize;
		int  		size;

		void fillSpace(int start, int size, char val);
};

int linearAddr(int addr, MEMORY_TYPE type);
int regularAddr(int addr, MEMORY_TYPE type);

#endif