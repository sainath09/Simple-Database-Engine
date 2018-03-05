#ifndef SORT_H
#define SORT_H

#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Heap.h"
#include "Pipe.h"
#include "BigQ.h"
#include "AbstractDBFile.h"

#define PIPE_BUFFER 100

typedef struct SortInfo
{
	OrderMaker *myOrder;
	int runLength;
} SortInfoDef;

typedef enum { READ,WRITE} RWMode;

// stub DBFile header..replace it with your own DBFile.h
class Sort : virtual public AbstractDBFile
{
  private:
	OrderMaker *order; // may come from startup or meta file; need to differentiate
	int runLength;
	fType type;
	const char *fpath;

	Page *pageBuffer;
	Pipe *iPipe;
	Pipe *oPipe;
	BigQ *biq;
	structBigQ *stBigQ;
	pthread_t diffrentialThread;
	Heap *heapDB;
	Heap resultHeap;
	RWMode rwmode;

	bool buildNewQuery;
	bool queryBuildable;
	OrderMaker *query;
	Page midPage;

  public:
	Sort();
	~Sort();
	int Create(const char *fpath, fType file_type, void *startup);
	int Open(const char *fpath);
	int Close();
	void Load(Schema &myschema, const char *loadpath);
	void MoveFirst();
	void Add(Record &addme);
	int GetNext(Record &fetchme);
	int GetNext(Record &fetchme, CNF &cnf, Record &literal);
	bool binarySearch(Record& fetchMe,CNF &cnf,Record& literal);
	int GetNextQuery(Record &fetchme, CNF &cnf, Record &literal);
	void toggleRW();
	void mergeDB();
};
#endif
