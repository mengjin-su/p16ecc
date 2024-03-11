#ifndef P16LINK_H
#define P16LINK_H

#include "fcall.h"

enum {
	RAM_LOC_ABS,
	RAM_LOC_LINEAR,
	RAM_LOC_FLOAT,
	ROM_LOC_ABS,
	ROM_LOC_FLOAT,
};

class P16link {

	private:
		Memory 	*codeMem;
		Memory 	*dataMem;
		Symbol 	*symbList;
		FILE 	*fout;
		FcallMgr*fcallMgr;
		
	public:
		int 	errorCount;

	public:
		P16link(Memory *ram, Memory *rom);
		~P16link();

		bool scanInclusion(void);
		void assignSegmentsAddress(void);
		void outputHex(char *filename);
		void outputMap(char *filename);

	private:
		// p16link.cpp
		void searchUsedSegment(Symbol *slist);
		bool searchUsedSegment(Segment *segp, item_t *ip, Symbol *slist);
		void removeUnusedSegment(Segment **segp);

		// p16link1.cpp
		void scanFuncLocalData(void);
		void logSegmentSymbols(Segment *seglist, Symbol **symlist, bool incl_func_name=false);
		void assignFuncLocalData(Symbol **symlist);

		// p16link2.cpp
		void assignSegmentMem(Segment *sp, int mode);
		void assignSegmentAddr(Memory *mem, Segment *seg, int addr, int flag);
		bool confirmSegmentMem(void);
		
		// p16link4.cpp
		void scanFcall(Segment *segp);
};

#endif