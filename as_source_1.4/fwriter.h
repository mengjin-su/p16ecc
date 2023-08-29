#ifndef _FILEWRITER_H
#define _FILEWRITER_H

#define MAX_OBJ_OUTPUT_WIDTH	80
#define MAX_LST_OUTPUT_WIDTH	24

class ObjWriter {
	private:
		FILE 	*fout;
		char 	lineType;
		int 	columnNum;
		char	buf[4096];
	public:
		ObjWriter(FILE *fout);
		void output(char type, int value);
		void output(char type, char *str);
		void output(char *str);
		void output(const char *str);
		void output(int val);
		void outputW(int code, char *str=NULL, int mask=0);
		void outputW(int code, int *val, int mask=0);
		void outputW(char *str, int mask);
		void flush(void);
		void close(void);
};

class LstWriter {
	private:
		FILE *fout;
		int  columnNum;
		char lineType;
		line_t *curLine;
		line_t *prtLine;

	public:
		LstWriter(FILE *fout);
		void flush(void);
		void output(unsigned int addr, int code);
		void output(unsigned int addr, int code, char type);
		void output(line_t *lp);
		void close(void);
};

#endif // end of _FILEWRITER_H