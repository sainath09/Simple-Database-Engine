
CC = g++  -Wall -w -std=c++11 -O2 -Wno-deprecated

DIR = $(shell pwd)
BIN = $(DIR)/bin
SRC = $(DIR)/src

tag = -i

ifdef linux
tag = -n
endif
main:             Compiler.o Catalog.o Optimizer.o Execute.o Parser.o Statistics.o Function.o Comparison.o Sort.o Heap.o AbstractDBFile.o Record.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o RelOp.o y.tab.o lex.yy.o  main.o
	$(CC) -o main Compiler.o Catalog.o Optimizer.o Execute.o Parser.o Statistics.o Function.o Comparison.o Sort.o Heap.o AbstractDBFile.o Record.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o RelOp.o y.tab.o lex.yy.o  main.o -lfl -lpthread
# main:   Compiler.o Catalog.o Optimizer.o Execute.o Parser.o  Parser.o Statistics.o Function.o Comparison.o Sort.o Heap.o AbstractDBFile.o Record.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o yyfunc.tab.o  lex.yyfunc.o main.o
# 	$(CC) -o main Compiler.o Catalog.o Optimizer.o Execute.o Parser.o Statistics.o Function.o Comparison.o Sort.o Heap.o AbstractDBFile.o Record.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o yyfunc.tab.o  lex.yyfunc.o main.o -lfl -lpthread
	mv *.o $(BIN)
	mv main $(BIN)

Compiler.o : $(SRC)/Compiler.cc
	$(CC) -g -c $(SRC)/Compiler.cc

Catalog.o : $(SRC)/Catalog.cc
	$(CC) -g -c $(SRC)/Catalog.cc

Parser.o : $(SRC)/Parser.cc
	$(CC) -g -c $(SRC)/Parser.cc

Optimizer.o : $(SRC)/Optimizer.cc
	$(CC) -g -c $(SRC)/Optimizer.cc

Execute.o : $(SRC)/Execute.cc
	$(CC) -g -c $(SRC)/Execute.cc

main.o : $(SRC)/main.cc
	$(CC) -g -c $(SRC)/main.cc

test.out:  Statistics.o y.tab.o lex.yy.o test.o
	$(CC) -o test.out   Statistics.o y.tab.o lex.yy.o test.o -lfl -lpthread
	
# a2-2test.out: Sort.o Heap.o AbstractDBFile.o Record.o Comparison.o RelOp.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a2-2test.o
# 	$(CC) -o a2-2test.out Sort.o Heap.o AbstractDBFile.o Record.o RelOp.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a2-2test.o -lfl -lpthread
	
# a21test.out: Heap.o Sort.o AbstractDBFile.o  Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a21test.o
# 	$(CC) -o a21test.out Heap.o Sort.o AbstractDBFile.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o BigQ.o DBFile.o Pipe.o y.tab.o lex.yy.o a21test.o -lfl -lpthread
	
# a1test.out: Heap.o Sort.o AbstractDBFile.o BigQ.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Pipe.o y.tab.o lex.yy.o a1test.o
# 	$(CC) -o a1test.out Heap.o Sort.o AbstractDBFile.o BigQ.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o Pipe.o y.tab.o lex.yy.o a1test.o -lfl -lpthread
	
test.o: $(SRC)/test.cc
	$(CC) -g -c $(SRC)/test.cc

Statistics.o: $(SRC)/Statistics.cc
	$(CC) -g -c $(SRC)/Statistics.cc
	
a2-2test.o: $(SRC)/a2-2test.cc
	$(CC) -g -c $(SRC)/a2-2test.cc

a21test.o: $(SRC)/a21test.cc
	$(CC) -g -c $(SRC)/a21test.cc

a1test.o: $(SRC)/a1test.cc
	$(CC) -g -c $(SRC)/a1test.cc

Sort.o: $(SRC)/Sort.cc
	$(CC) -g -c $(SRC)/Sort.cc

Heap.o: $(SRC)/Heap.cc
	$(CC) -g -c $(SRC)/Heap.cc

Comparison.o: $(SRC)/Comparison.cc
	$(CC) -g -c $(SRC)/Comparison.cc
	
ComparisonEngine.o: $(SRC)/ComparisonEngine.cc
	$(CC) -g -c $(SRC)/ComparisonEngine.cc
	
DBFile.o: $(SRC)/DBFile.cc
	$(CC) -g -c $(SRC)/DBFile.cc

Pipe.o: $(SRC)/Pipe.cc
	$(CC) -g -c $(SRC)/Pipe.cc

BigQ.o: $(SRC)/BigQ.cc
	$(CC) -g -c $(SRC)/BigQ.cc

RelOp.o: $(SRC)/RelOp.cc
	$(CC) -g -c $(SRC)/RelOp.cc

Function.o: $(SRC)/Function.cc
	$(CC) -g -c $(SRC)/Function.cc

File.o: $(SRC)/File.cc
	$(CC) -g -c $(SRC)/File.cc

Record.o: $(SRC)/Record.cc
	$(CC) -g -c $(SRC)/Record.cc

Schema.o: $(SRC)/Schema.cc
	$(CC) -g -c $(SRC)/Schema.cc

AbstractDBFile.o: $(SRC)/AbstractDBFile.cc
	$(CC) -g -c $(SRC)/AbstractDBFile.cc
	
y.tab.o: $(SRC)/Parser.y
	yacc -p "yy" -b "y" -d $(SRC)/Parser.y
	sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c y.tab.c
		
lex.yy.o: $(SRC)/Lexer.l
	lex $(SRC)/Lexer.l
	gcc  -c lex.yy.c -o lex.yy.o

clean: 
	rm -f $(BIN)/main
	rm -f $(BIN)/*.o
	rm -f $(BIN)/*.out
	rm -f main
	rm -f *.o
	rm -f *.out
	rm -f y.tab.c
	rm -f lex.yy.c
	rm -f y.tab.h
	rm -f *.bin

cleanDB:
	rm -f $(BIN)/catalog
	rm -f $(BIN)/*.bin
	rm -f $(BIN)/*.bin.METAINF
