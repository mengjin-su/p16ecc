#ifndef FWRITER_MAP_H
#define FWRITER_MAP_H
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "symbol.h"
#include "segment.h"

class MapWriter {
	public:
		FILE *fout;
		
	public:
		MapWriter(char *filename);
		void close(void);
		void outputSeg(Symbol *slist, int used, int total);
		void outputSeg(Segment *seg, Symbol *slist, int used, int total);
		void outputInst(int addr, int inst);
		void outputSkip(void);
		void outputSkip0(void);
};

#endif