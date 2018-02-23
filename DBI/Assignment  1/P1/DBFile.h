#ifndef DBFILE_H
#define DBFILE_H
#include "AbstractDBFile.h"
#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"





// stub DBFile header..replace it with your own DBFile.h 

class DBFile {
private:
	AbstractDBFile *AbsDBFile;
public:
	DBFile (); 
	int Create ( char *fpath, fType file_type, void *startup);
	int Open ( char *fpath);
	int Close ();
	void Load (Schema &myschema, char *loadpath);
	void MoveFirst ();
	void Add (Record &addme);
	int GetNext (Record &fetchme);
	int GetNext (Record &fetchme, CNF &cnf, Record &literal);

};
#endif
