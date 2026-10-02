CC  = gcc
CXX = g++
RM  = rm
MV  = mv
CP  = cp
EXE = st8cc.exe
OBJ = main.o lex.yy.o cc.o common.o ascii.o option.o nlist.o prescan.o sizer.o \
	display.o dnode.o dlink.o flink.o item.o pnode.o const.o pcoder.o \
	pcoder1.o pcoder2.o pcoder3.o pcoder4.o pcoder5.o pcoder6.o pcoder7.o \
	pcoder8.o pcoder9.o popt.o popt1.o popt2.o popt3.o popt4.o popt5.o popt6.o \
	stm8s.o stm8s_reg.o stm8s_asm.o stm8s0.o stm8s1.o stm8s2.o stm8s3.o stm8s4.o \
	stm8s5.o stm8s6.o stm8s7.o stm8s8.o stm8s9.o stm8s10.o stm8s11.o stm8s12.o stm8s13.o stm8s14.o
	
OPTIONS= -c -Os

$(EXE): $(OBJ) cc.h makefile
	$(CXX) -static $(OBJ) -o $(EXE)
	$(CP) $(EXE) ../bin
	
%.o: %.c cc.h makefile
	$(CC) $(OPTIONS) $<

%.o: %.cpp cc.h makefile
	$(CXX) $(OPTIONS) $<

%.o: ./stm8/%.cpp cc.h makefile
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
