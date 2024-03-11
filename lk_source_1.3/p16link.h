#ifndef P16LINK_H
#define P16LINK_H

#include "fcall.h"

enum {
	RAM_LOC_ABS,
	RAM_LOC_BANK,
	RAM_LOC_FLOAT,
	ROM_LOC_ABS,
	ROM_LOC_FLOAT,
};

class P16link {

	private:
		Memory  	*codeMem;
		Memory  	*dataMem;
		Symbol  	*symbList;
		FcallMgr	*fcallHandler;
		FILE 		*fout;
	public:
		int errorCount;

	public:
		P16link(Memory *ram, Memory *rom);
		~P16link();

		void print(FILE *fo=NULL);
		void print(Segment *);
		void print(line_t *);
		void print(item_t *, int c=0);

		// p16link1.cpp
		int  scanLibInclusion(void);
		int  scanSegmentGroups(void);
		void logDataSegLabels(Symbol **slist, Segment *seg);
		int  confirmSegmentMem(void);
		void removeExtraJKLines(void);

		// p16link4.cpp
		void outputHex(char *filename);

		// p16link5.cpp
		void outputMap(char *filename, Memory *);

		// p16link7.cpp
		void scanFuncCalls(void);

	private:	// p16link0.cpp
		Segment *extractLibSegment(Segment **slist, char *lbl, Segment *seg);
		void searchUnusedSegment(Symbol *slist);
		bool searchUnusedSegment(Segment *segp, item_t *ip, Symbol *slist);
		void removeUnusedSegment(Segment **segp);

	private:
		void logSegLabels(Segment *seglist, Symbol **symlist, int mode);
		bool scanSegLabels(Segment *seglist, Symbol *symlist);
		void scanFuncLocalData(Symbol **symlist);

		void scanSegmentGroup(Segment *sp, int flag);
		void assignSegmentMem(Memory *mem, Segment *seg, int addr, int flag);
};

#endif