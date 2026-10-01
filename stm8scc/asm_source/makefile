CC = gcc
GXX= g++
CP = cp
MV = ren
RM = rm
OBJ = main.o asm.o lex.yy.o common.o symbol.o parse.o seg.o \
      exp.o st8inst.o st8asm.o asmout.o
      
OPTIONS= -c -Os
            
EXE = st8as.exe

$(EXE): $(OBJ) asm.h makefile
	$(GXX) $(OBJ) -static -o $(EXE)
	$(CP) $(EXE) ../bin

%.o: %.cpp asm.h
	$(GXX) $(OPTIONS) $<
	
%.o: %.c asm.h
	$(CC) $(OPTIONS) $<
	
asm.c: asm.y makefile
	bison -d asm.y -o asm.c
	
asm.h: asm.y makefile
	bison -d asm.y -o asm.c	
	
lex.yy.c: asm.l asm.h makefile
	flex asm.l

clean:
	$(RM) *.o
	$(RM) asm.h
	$(RM) asm.c
	$(RM) lex.yy.*
	$(RM) $(EXE)

