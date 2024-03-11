CC  = gcc
CXX = g++
RM  = rm
MV  = mv
CP  = cp
EXE = cc16e.exe
OBJ = main.o lex.yy.o cc.o common.o display.o ascii.o nlist.o sizer.o prescan.o \
      item.o pnode.o dlink.o flink.o pcoder.o const.o pcoder1.o pcoder2.o pcoder3.o \
	  pcoder4.o pcoder5.o pcoder6.o pcoder7.o pcoder8.o popt.o popt1.o popt2.o popt3.o \
	  popt4.o popt5.o popt6.o \
	  pic16e_asm.o pic16e.o pic16e_reg.o pic16e0.o pic16e1.o pic16e2.o pic16e3.o pic16e4.o \
	  pic16e5.o pic16e6.o pic16e7.o pic16e8.o pic16e9.o pic16e10.o pic16e11.o

OPTIONS= -c -Os -Wall

$(EXE): $(OBJ) cc.h makefile
	$(CXX) -static $(OBJ) -o $(EXE)
	$(CP) $(EXE) ../bin
	
%.o: %.c cc.h makefile
	$(CC) $(OPTIONS) $<

%.o: %.cpp cc.h makefile
	$(CXX) $(OPTIONS) $<
	
%.o: ./p16e/%.cpp cc.h makefile
	$(CXX) $(OPTIONS) $<
	
lex.yy.c: cc.l cc.h makefile
	flex cc.l

cc.h: cc.y makefile
	bison -d cc.y -o cc.c

cc.c: cc.y makefile
	bison -d cc.y -o cc.c
	
clean:
	$(RM) *.o
	$(RM) cc.h
	$(RM) cc.c
	$(RM) lex.yy.c
	$(RM) $(EXE)
