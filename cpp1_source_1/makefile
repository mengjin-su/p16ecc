CC  = g++
RM  = rm
CP  = cp

EXE = cpp1.exe
OBJ = main.o cpp1.o
	
OPTIONS= -c -Wall -Os 

$(EXE): $(OBJ) makefile
	$(CC) -static $(OBJ) -o $(EXE)
	$(CP) $(EXE) ../bin	
	
%.o: %.cpp makefile
	$(CC) $(OPTIONS) $<

clean:
	$(RM) *.o
	$(RM) $(EXE)

