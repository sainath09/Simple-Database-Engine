CC = g++ -O2 -Wno-deprecated 

tag = -i

ifdef linux
tag = -n
endif

test.out: RelOp.o  Sort.o BigQ.o Pipe.o Function.o AbstractDBFile.o Heap.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test.o
	$(CC) -pthread -o test.out RelOp.o  Sort.o Pipe.o BigQ.o Function.o AbstractDBFile.o Heap.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o test.o -lfl

a21test.out:Sort.o  Pipe.o BigQ.o AbstractDBFile.o Function.o Heap.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o a21test.o
	$(CC) -pthread -o Sort.o Pipe.o BigQ.o a21test.out AbstractDBFile.o Heap.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o a1test.o -lfl
	
a1test.out:Sort.o BigQ.o Pipe.o AbstractDBFile.o Heap.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o a1test.o
	$(CC) -pthread -o a1test.out Sort.o Pipe.o BigQ.o AbstractDBFile.o Heap.o Record.o Comparison.o ComparisonEngine.o Schema.o File.o DBFile.o y.tab.o lex.yy.o a1test.o -lfl


a21test.o: a21test.cc
	$(CC) -g -c a21test.cc

a1test.o: a1test.cc
	$(CC) -g -c a1test.cc	

test.o: test.cc
	$(CC) -g -c test.cc
RelOp.o: RelOp.cc
	$(CC) -g -c RelOp.cc
Sort.o: Sort.cc
	$(CC) -g -c Sort.cc
Pipe.o : Pipe.cc
	$(CC)  -g -c Pipe.cc
BigQ.o : BigQ.cc
	$(CC) -g -c BigQ.cc
Heap.o: Heap.cc
	$(CC) -g -c Heap.cc

main.o: main.cc
	$(CC) -g -c main.cc
	
Comparison.o: Comparison.cc
	$(CC) -g -c Comparison.cc
	
ComparisonEngine.o: ComparisonEngine.cc
	$(CC) -g -c ComparisonEngine.cc
	
DBFile.o: DBFile.cc
	$(CC) -g -c DBFile.cc

File.o: File.cc
	$(CC) -g -c File.cc

Record.o: Record.cc
	$(CC) -g -c Record.cc

Schema.o: Schema.cc
	$(CC) -g -c Schema.cc
	
y.tab.o: Parser.y
	yacc -d Parser.y
	sed $(tag) y.tab.c -e "s/  __attribute__ ((__unused__))$$/# ifndef __cplusplus\n  __attribute__ ((__unused__));\n# endif/" 
	g++ -c y.tab.c

lex.yy.o: Lexer.l 
	lex  Lexer.l
	gcc  -c lex.yy.c

clean: 
	rm -f *.o
	rm -f *.out
	rm -f y.tab.c
	rm -f y.tab.c-e
	rm -f lex.yy.c
	rm -f y.tab.h
	rm -f *.bin
	rm -f *.METAINF
