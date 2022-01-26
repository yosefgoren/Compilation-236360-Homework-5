.PHONY: all clean

all: clean
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -o hw5 *.c *.cpp
clean:
	rm -f lex.yy.c
	rm -f parser.tab.*pp
	rm -f hw5

tar:
	zip 211515606-317580900 scanner.lex parser.ypp hw3_output.hpp hw3_output.cpp bp.hpp bp.cpp Symtab.hpp Symtab.cpp AuxTypes.cpp AuxTypes.hpp

COMP_FLAGS=-std=c++17

bdn:
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -g3 -o hw5 *.c *.cpp

bdo:
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -g3  -DOLDT -o hw5 *.c *.cpp
