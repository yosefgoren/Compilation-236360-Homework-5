.PHONY: all clean

all: clean
	flex fscanner.lex
	bison -Wcounterexamples -d fparser.ypp
	g++ -std=c++17 -o free *.c *.cpp ../bp.cpp
clean:
	rm -f lex.yy.c
	rm -f fparser.tab.*pp
	rm -f free

test: all
	./free < in1 > out1.llvm
	lli out1.llvm