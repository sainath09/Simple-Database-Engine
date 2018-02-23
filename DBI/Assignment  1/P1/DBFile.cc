
// #include "TwoWayList.h"
//#include "Record.h"
//#include "Schema.h"
//#include "File.h"
//#include "Comparison.h"
//#include "ComparisonEngine.h"
#include "DBFile.h"
#include "AbstractDBFile.h"
#include "Heap.h"
#include <string>
#include <fstream>
#include <iostream>
DBFile::DBFile () {
}

int DBFile::Create ( char *f_path, fType f_type, void *startup) {
    //Create a file object
    DBFile DBFile;
    string metaFile(f_path); 
     //create a meta file with some sort of METAINF tag using fstreams
    metaFile +=  ".METAINF";
    ofstream out;
    out.open(metaFile.c_str(),std::ofstream::out);
     if(f_type==heap){
         //CREATE NEW HEAP CLASS OBJECT INTO A GENERIC DB FILE. 
            AbsDBFile = new Heap();
            out<<"heap";
            //store type of DBFile .. heap, b+, or anything else in meta
     }
     else {
         //work for sorted 
         out<<"sorted";
     }
     out.close();
     //CALL THE FUNCTION "CREATE" FOR THE HEAPDB OBJECT
    return AbsDBFile->Create(f_path,f_type,startup);
}

void DBFile::Load(Schema &f_schema,  char *loadpath) {
    AbsDBFile->Load(f_schema,loadpath);
}
int DBFile::Open( char *f_path) {
    ifstream in;
    string metaFile(f_path);
    metaFile += ".METAINF";
    in.open(metaFile.c_str(),ifstream::in);
    if(!in.is_open()){
        cout<<"meta file not created. Please check the create method";
    }
    else{
        string type = "";
        getline(in,type);
        if(type == "heap"){
            AbsDBFile = new Heap();
        }
        else if(type == "sorted"){
            cout<<"implemented in sorted DS";

        }
        else {
            cout<<"to be implemented";
        }
    }
    return AbsDBFile->Open(f_path);

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
