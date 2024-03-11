CC  = gcc
RM  = rm

EXE = cpp1.exe
OBJ = lex.yy.o
	
OPTIONS= -c -Os 

$(EXE): $(OBJ) makefile
	$(CC) -static $(OBJ) -o $(EXE)
	cp $(EXE) ../bin
	
%.o: %.c makefile
	$(CC) $(OPTIONS) $<

lex.yy.c: cpp.l makefile
	flex cpp.l

clean:
	$(RM) *.o
	$(RM) lex.yy.c
	$(RM) $(EXE)
	
