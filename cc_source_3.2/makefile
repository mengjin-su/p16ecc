CC  = gcc
CXX = g++
RM  = rm
CP  = cp
EXE = cc16e.exe
OBJ = main.o lex.yy.o cc.o

OPTIONS= -c -Os

$(EXE): $(OBJ) cc.h makefile
	$(CXX) -static $(OBJ) -o $(EXE)
	$(CP) $(EXE) ../bin
	
%.o: %.c cc.h makefile
	$(CC) $(OPTIONS) $<

%.o: %.cpp makefile
	$(CXX) $(OPTIONS) $<
	
lex.yy.c: cc.l cc.h makefile
	flex cc.l
	
cc.h: cc.y makefile
	bison -d cc.y -o cc.c

cc.c: cc.y makefile
	bison -d cc.y -o cc.c	
	
clean:
	$(RM) *.o
	$(RM) lex.yy.c
	$(RM) cc.h
	$(RM) cc.c
	$(RM) $(EXE)
