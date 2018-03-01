#ifndef SORT_H
#define SORT_H

#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Pipe.h"
#include "BigQ.h"
#include "AbstractDBFile.h"

#define PIPE_BUFFER 100

typedef struct SortInfo {
	OrderMaker *myOrder;
	int runLength;
}SortInfoDef; 

enum Mode {READ, WRITE} mode;

// stub DBFile header..replace it with your own DBFile.h 
class Sort: virtual public AbstractDBFile {
private: 
	OrderMaker* order;    // may come from startup or meta file; need to differentiate
  	int runLength;
	fType type;
	
    File *refFile;
    Page *wBuffer, *rBuffer;
	Pipe *iPipe;
	Pipe *oPipe;
  	BigQ *biq;
    
    int currPageInd; //index of file to read

public:
	Sort (); 
    ~Sort (); 
	int Create (const char *fpath, fType file_type,void *startup);
	int Open (const char *fpath);
	int Close ();
	void Load (Schema &myschema, const char *loadpath);
	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);
	void toggleRW();
};
#endif
