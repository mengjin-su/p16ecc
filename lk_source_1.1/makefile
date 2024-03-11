CC  = gcc
CXX = g++
RM  = rm
MV  = mv
CP  = cp
EXE = lk16e.exe
OBJ = main.o lex.yy.o lnk.o common.o path.o segment.o

OPTIONS= -c -Os

$(EXE): $(OBJ) lnk.h makefile
	$(CXX) -static $(OBJ) -o $(EXE)
	$(CP) $(EXE) ../bin
	
%.o: %.c lnk.h makefile
	$(CC) $(OPTIONS) $<

%.o: %.cpp lnk.h makefile
	$(CXX) $(OPTIONS) $<

lex.yy.c: lnk.l lnk.h makefile
	flex lnk.l

lnk.h: lnk.y makefile
	bison -d lnk.y -o lnk.c

lnk.c: lnk.y makefile
	bison -d lnk.y -o lnk.c
	
clean:
	$(RM) *.o
	$(RM) lnk.h
	$(RM) lnk.c
	$(RM) lex.yy.c
	$(RM) $(EXE)
