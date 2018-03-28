#ifndef REL_OP_H
#define REL_OP_H

#include "Pipe.h"
#include "DBFile.h"
#include "Record.h"
#include "Function.h"
#include "BigQ.h"
#include "Sort.h"
#include <sstream>


//structure to start a selectpipe thread
typedef struct{
	Pipe* inPipe;
	Pipe* outPipe;
	CNF* selOp;
	Record* literal;
}stSelPipe;
//structure to start a selectfile thread
typedef struct{
	DBFile* inFile;
	Pipe* outPipe;
	CNF* selOp;
	Record* literal;
}stSelFile;
//structure to start a project thread
typedef struct{
	Pipe* inPipe;
	Pipe* outPipe;
	int* keepMe;
	int numAttsInput;
	int numAttsOutput;
}stProject;
//structure to start a join thread
typedef struct{
	Pipe* inPipeL;
	Pipe* inPipeR;
	Pipe* outPipe;
	CNF* selOp;
	Record* literal;
}stJoin;
//structure to start a duplicate removal thread
typedef struct{
	Pipe* inPipe;
	Pipe* outPipe;
	Schema* mySchema;
}stDupRem;
//structure to start a sum thread
typedef struct{
	Pipe* inPipe;
	Pipe* outPipe;
	Function* computeMe;
}stSum;
//structure to start a GroupBy thread
typedef struct{
	Pipe* inPipe;
	Pipe* outPipe;
	OrderMaker* groupAtts;
	Function* computeMe;
}stGroupBy;
//structure to start a writeout thread
typedef struct{
	Pipe* inPipe;
	FILE* outFile;
	Schema* mySchema;
}stWOut;

class RelationalOp {
	public:
	// blocks the caller until the particular relational operator 
	// has run to completion
	virtual void WaitUntilDone () = 0;

	// tell us how much internal memory the operation can use
	virtual void Use_n_Pages (int n) = 0;
};

class SelectFile : public RelationalOp { 

	private:
		pthread_t thread;
	// Record *buffer;
		int pages;

	public:

	void Run (DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal);
	void WaitUntilDone ();
	void Use_n_Pages (int n);

};

class SelectPipe : public RelationalOp {
	private:
		pthread_t thread;
		int pages;
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal);
	void WaitUntilDone () ;
	void Use_n_Pages (int n) ;

};
class Project : public RelationalOp { 
	private:
		pthread_t thread;
		int pages;
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput);
	void WaitUntilDone () ;
	void Use_n_Pages (int n) ;
};
class Join : public RelationalOp { 
	private:
		pthread_t thread;
		int pages;
	public:
	void Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal);
	void WaitUntilDone () ;
	void Use_n_Pages (int n) ;
};
class DuplicateRemoval : public RelationalOp {
	private:
		pthread_t thread;
		int pages;
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema);
	void WaitUntilDone () ;
	void Use_n_Pages (int n) ;
};
class Sum : public RelationalOp {
	private:
		pthread_t thread;
		int pages;
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe);
	void WaitUntilDone () ;
	void Use_n_Pages (int n) ;
};
class GroupBy : public RelationalOp {
	private:
		pthread_t thread;
		int pages;
	public:
	void Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);
	void WaitUntilDone () ;
	void Use_n_Pages (int n) ;
};
class WriteOut : public RelationalOp {
	private:
		pthread_t thread;
		int pages;
	public:
	void Run (Pipe &inPipe, FILE *outFile, Schema &mySchema) ;
	void WaitUntilDone () ;
	void Use_n_Pages (int n) ;
};
#endif
