#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "AbstractDBFile.h"
#include "Defs.h"
#include "Heap.h"

DBFile::DBFile () {
}

//TODO: 
int DBFile::Create (const char *f_path, fType f_type, void *startup) {
    //Create a file object
    DBFile DBFile;
    //Check if meta data object exists, else create new
    //create a meta file with some sort of METAINF tag using Stringstream (#include sstream)
    
    //File open with 0 would create a new file.
    // DBFile.file.Open(0, (char *)f_path);
    //Create some error handling in case DBFIle cannot be created.

     if(f_type==heap){
         //CREATE NEW HEAP CLASS OBJECT INTO A GENERIC DB FILE. 
            AbsDBFile = new Heap();
            //store type of DBFile .. heap, b+, or anything else.

          //Put first position of FILEH Record in MetaData file

     }
     //CALL THE FUNCTION "CREATE" FOR THE HEAPDB OBJECT
    return AbsDBFile->Create(f_path,f_type,startup);

}

void DBFile::Load (Schema &f_schema, const char *loadpath) {
    AbsDBFile->Load(f_schema,loadpath);
}
//TODO: 
int DBFile::Open (const char *f_path) {
}

void DBFile::MoveFirst () {
    AbsDBFile->MoveFirst();
}

int DBFile::Close () {
    return AbsDBFile->Close();
}

void DBFile::Add (Record &rec) {
    AbsDBFile->Add(rec);
}

int DBFile::GetNext (Record &fetchme) {
   return AbsDBFile->GetNext(fetchme);
}

int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
   return AbsDBFile->GetNext(fetchme,cnf,literal);
}
