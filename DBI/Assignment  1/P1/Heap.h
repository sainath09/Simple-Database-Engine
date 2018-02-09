#ifndef HEAP_H
#define HEAP_H

#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "AbstractDBFile.h"
#include "DBFile.h"
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
	int Create (const char *fpath, fType file_type, void *startup);
	int Open (const char *fpath);
	int Close ();
	void Load (Schema &myschema, const char *loadpath);
	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);

};
#endif
