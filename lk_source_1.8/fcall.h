#ifndef _FCALL_H
#define _FCALL_H

enum {MAIN_FUNC=1, ISR_FUNC, SUB_FUNC, NULL_FUNC};

#define BNODE(i,j)		(basicVectorMatrix[i][j])
#define ENODE(i,j)		(extendVectorMatrix[i][j])

typedef enum {IS_CALLER, IS_CALLEE} FUNC_MODE;

typedef struct _Fcall {
	char	*fileName;
	char	*caller;
	char	*callee;
	struct _Fcall *next;
} Fcall;

typedef struct _FuncAttr {
	char	*fileName;	// file name
    char	*funcName;	// function name
	int		 dataSize;	// function local data size
	int		 funcIndex;	// internal index
	bool	 global;
	Segment	*segment;
	char    *rootList;
	int     funcType;
	bool	standalone;
    struct _FuncAttr *next;
} FuncAttr;

class FcallMgr {

	private:
		FuncAttr *funcList;
		char 	 **basicVectorMatrix;
		char	 **extendVectorMatrix;

	public:
		Fcall	 *fcallList;
		int 	 funcCount;

	public:
		FcallMgr();
		~FcallMgr();

		void addFcallName(line_t *lp, Segment *seg);
		void addFcallLink(line_t *lp);
		void setGlobal(line_t *lp, char *func);

		void createBasicMatrix(void);
		void createExtendMatrix(void);

		FuncAttr *getFunc(int f_idex);
		FuncAttr *getFunc(char *file, char *func, FUNC_MODE mode);

		// fcall1.cpp
		int  funcType(int f_index);
		void findRoot(int f_index, char *trace_list, char *root_list);
		int  funcDataSize(int f_index);
		bool joinInGroup(int *group, int length, int f_index);

		void outputCallPath(FILE *fout=NULL);
		void outputCallPath(FILE *fout, int *queue, int length);

	private:
		Fcall *searchFcall(char *fname, char *caller, char *callee);
		void validation(int f_index);
};


#endif