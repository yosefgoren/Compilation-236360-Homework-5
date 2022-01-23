.PHONY: all clean

COMP_FLAGS=-std=c++17

# hw5: AuxTypes.o bp.o hw3_output.o parser.tab.o Symtab.o lex.yy.o
# 	ld -o $@ $^
# parser.tab.cpp:
# 	bison -Wcounterexamples -d parser.ypp
# yylex.yy.c: scanner.lex parser.tab.hpp Symtab.hpp
# 	flex scanner.lex
# parser.tab.hpp: parser.tab.cpp

# AuxTypes.o: AuxTypes.cpp AuxTypes.hpp bp.hpp
# 	g++ ${COMP_FLAGS} -c $^
# bp.o: bp.cpp bp.hpp AuxTypes.hpp Symtab.hpp
# 	g++ ${COMP_FLAGS} -c $^
# hw3_output.o: hw3_output.cpp hw3_output.hpp
# 	g++ ${COMP_FLAGS} -c $^
# parser.tab.o: parser.tab.cpp Symtab.hpp AuxTypes.hpp hw3_output.hpp bp.hpp parser.tab.hpp
# 	g++ ${COMP_FLAGS} -c $^
# Symtab.o: Symtab.cpp Symtab.hpp AuxTypes.hpp hw3_output.hpp
# 	g++ ${COMP_FLAGS} -c $^
# lex.yy.o: lex.yy.c hw3_output.hpp Symtab.hpp AuxTypes.hpp parser.tab.hpp
# 	g++ ${COMP_FLAGS} -c $^

bdn:
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -g3 -o hw5 *.c *.cpp

bdo:
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -DOLDT -o hw5 *.c *.cpp

all: clean
	flex scanner.lex
	bison -Wcounterexamples -d parser.ypp
	g++ -std=c++17 -o hw5 *.c *.cpp
clean:
	rm -f lex.yy.c
	rm -f parser.tab.*pp
	rm -f hw5
