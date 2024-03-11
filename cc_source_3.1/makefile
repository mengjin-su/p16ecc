CC  = gcc
CXX = g++
RM  = rm
MV  = mv
CP  = cp
EXE = cc16e.exe
OBJ = main.o lex.yy.o

OPTIONS= -c -Os

$(EXE): $(OBJ) makefile
	$(CXX) -static $(OBJ) -o $(EXE)
	$(CP) $(EXE) ../bin
	
%.o: %.c makefile
	$(CC) $(OPTIONS) $<

%.o: %.cpp makefile
	$(CXX) $(OPTIONS) $<
	
lex.yy.c: cc.l makefile
	flex cc.l
	
clean:
	$(RM) *.o
	$(RM) lex.yy.c
	$(RM) $(EXE)
