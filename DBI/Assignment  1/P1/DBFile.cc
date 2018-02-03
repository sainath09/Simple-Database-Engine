#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "Defs.h"

// stub file .. replace it with your own DBFile.cc

DBFile::DBFile () {
    //set the current file index to zero initially;
    currFileIndex  = 0 ;

}

int DBFile::Create (const char *f_path, fType f_type, void *startup) {
    //Create a file object
    DBFile DBFile;
    //Check if meta data object exists, else create new
 
    
    //File open with 0 would create a new file.
    DBFile.file.Open(0, (char *)f_path);
    //Create some error handling in case DBFIle cannot be created.

     if(f_type==heap){
            //store type of DBFile .. heap, b+, or anything else.
          //Put first position of FILEH Record in MetaData file

     }


}

void DBFile::Load (Schema &f_schema, const char *loadpath) {
}

int DBFile::Open (const char *f_path) {
}

void DBFile::MoveFirst () {
}

int DBFile::Close () {
}

void DBFile::Add (Record &rec) {
}

int DBFile::GetNext (Record &fetchme) {
}

int DBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
}
