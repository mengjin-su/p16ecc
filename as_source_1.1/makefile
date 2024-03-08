CC  = gcc
CXX = g++
RM  = rm
MV  = mv
CP  = cp
EXE = as16e.exe
OBJ = main.o lex.yy.o asm.o common.o
OPTIONS= -c -Os -Wall

$(EXE): $(OBJ) asm.h makefile
	$(CXX) -static $(OBJ) -o $(EXE)
	$(CP) $(EXE) ../bin
	
%.o: %.c asm.h makefile
	$(CC) $(OPTIONS) $<

%.o: %.cpp asm.h makefile
	$(CXX) $(OPTIONS) $<
	
lex.yy.c: asm.l asm.h makefile
	flex asm.l

asm.h: asm.y makefile
	bison -d asm.y -o asm.c

asm.c: asm.y makefile
	bison -d asm.y -o asm.c
	
clean:
	$(RM) *.o
	$(RM) asm.h
	$(RM) asm.c
	$(RM) lex.yy.c
	$(RM) $(EXE)
