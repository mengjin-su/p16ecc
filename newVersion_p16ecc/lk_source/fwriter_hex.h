#ifndef FWRITER_H
#define FWRITER_H

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
		~HexWriter();
		
		void outputWord(int addr, int word);	// output an instruction word
		
	private:	
		void outputLine(void);					// output a data line
		void outputAddr(int addr);				// set Hi 16-bit address of a segment
};

#endif