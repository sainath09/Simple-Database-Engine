#ifndef HEAP_H
#define HEAP_H

#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "AbstractDBFile.h"

// stub DBFile header..replace it with your own DBFile.h 
class Heap: virtual public AbstractDBFile {
private: 
    File *refFile;
    Page *wBuffer;
    Page *rBuffer;
    fType type;
    int currPageInd; //index of file to read

public:
	Heap (); 
    ~Heap (); 
	int Create ( char *fpath, fType file_type,void *startup);
	int Open ( char *fpath);
	int Close ();
	void Load (Schema &myschema,  char *loadpath);
	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);

};
#endif
