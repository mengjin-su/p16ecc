#ifndef FWRITER_H
#define FWRITER_H
extern "C" {
#include "common.h"
#include "lnk.h"
}
#include "symbol.h"
#include "segment.h"

#define HEX_BUF_SIZE	128

class HexWriter {

	private:
		int startAddr;
		int currentAddr;
		int length;
		unsigned char buf[HEX_BUF_SIZE];
		FILE *fout;

	public:
		HexWriter(char *filename);
		void close(void);
		void outputWord(int addr, int word);
		void outputLine(void);
		void outputBank(int bank);
};

#endif