#ifndef _FCALL_H
#define _FCALL_H

#define FNODE(i,j)		(basicVectorMatrix[i][j])
#define ENODE(i,j)		(extendVectorMatrix[i][j])

typedef enum {IS_CALLER, IS_CALLEE} FUNC_MODE;

typedef struct _Fcall {
	char		  *fileName;
	char		  *caller;
	char		  *callee;
	struct _Fcall *next;
} Fcall;

typedef struct _FuncAttr {
	char			*fileName;	// file name 
    char 			*funcName;	// function name
    int				 dataSize;	// function local data size
	int 			 dataAddr;	// assigned address
	int				 funcIndex;	// internal index
	bool			 global;
	Segment			*segment;
    struct _FuncAttr *next;
} FuncAttr;

class FcallMgr {

	private:
		FuncAttr *funcList;
		char	 **extendVectorMatrix;
		char 	 **basicVectorMatrix;
		char 	 *primaryFunc;
		int 	 primaryCount;

	public:
		Fcall	 *fcallList;
		int 	 funcCount;

	public:
		FcallMgr();
		~FcallMgr();

		void clear(void);
		void addFunc(line_t *lp, Segment *seg);
		void addFcall(line_t *lp);
		void setGlobal(line_t *lp, char *func);		
		int  funcDataSize(int func_id);

		void printFunc(void);
		void printMatrix(void);

		void createShareGroups(void);
		bool testFuncCall(int *id_list, int length, int f_id);

		FuncAttr *searchFunc(int func_id);
		FuncAttr *searchFunc(char *file, char *func, FUNC_MODE mode);

		void outputCallPath(FILE *fout=NULL);
		void outputCallPath(FILE *fout, int *queue, int length);
		
	private:
		void createBasicMatrix(void);
		void createExtendMatrix(void);
		bool sameGroup(int f1, int f2);
		bool callRelated(int f1, int f2);
		bool withinGroup(int *id_list, int length, int f_id);
		int  withinPrimary(int fid);

		Fcall *searchFcall(char *fname, char *caller, char *callee);
};


#endif