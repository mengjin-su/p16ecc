#ifndef SEGMENT_H
#define SEGMENT_H

enum {
	CODE_SEGMENT,
	DATA_SEGMENT,
	CONST_SEGMENT,
	FUSE_SEGMENT,
	MISC_SEGMENT
};

class Segment {
	public:
		line_t	*lines;			// line pointer

		char    *fileName;		// file name
		bool	isLIB;			// lib code
		int		isBEG: 	1;		// begin of segment link
		int	 	isEND: 	1;		// end of segment link
		int	 	isREL: 	1;		// relocatable segment
		int	 	isABS: 	1;		// absolute address seg.
		int	 	isUsed: 1;		// code will be used
		int 	addr;			// ABS address
		int 	dataSize;		// function data size
		int 	maddr;			// memory mapped address
		int 	startAddr;

		Segment	*next;

	public:
		Segment(char *file_name);
		~Segment();
		void init(void);
		void addLine(line_t *lp);
		int  lineCount(void);
		int  type(void);		// segment type
		int  size(void);		// segment size
		char *name(void);		// segment name
		void print(void);
};

extern Segment	*libSegGroup;
extern Segment	*codeSegGroup;
extern Segment	*dataSegGroup;
extern Segment	*fuseSegGroup;
extern Segment	*miscSegGroup;

void addSegment(Segment *seg, int lib);
void deleteSegments(Segment *list);
void printfSegments(Segment *list);

void addSegment(Segment **slist, Segment *seg);

#endif