.PHONY: all clean

bdn:
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -g3 -std=c++17 -o hw5 *.c *.cpp

bdo:
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -g3 -std=c++17 -DOLDT -o hw5 *.c *.cpp

all: clean
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -o hw5 *.c *.cpp
clean:
	rm -f lex.yy.c
	rm -f parser.tab.*pp
	rm -f hw5
