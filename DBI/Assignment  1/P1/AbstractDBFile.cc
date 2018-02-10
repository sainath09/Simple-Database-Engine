#include <iostream>
#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "AbstractDBFile.h"


AbstractDBFile::~AbstractDBFile(){
}
AbstractDBFile::AbstractDBFile () {
}

int AbstractDBFile::Create (const char *fpath, fType file_type,void *startup){
    return 1;
}
    
void AbstractDBFile::Add (Record &rec) {
}

void AbstractDBFile::Load (Schema &f_schema, const char *loadpath) {
}

int AbstractDBFile::Open (const char *f_path) {
    return 1;
}

void AbstractDBFile::MoveFirst () {
}

int AbstractDBFile::Close () {
	return 0;
}

int AbstractDBFile::GetNext (Record &fetchme) {
	return 0;
}

int AbstractDBFile::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
	return 0;
}