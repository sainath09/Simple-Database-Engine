
// #include "TwoWayList.h"
//#include "Record.h"
//#include "Schema.h"
//#include "File.h"
//#include "Comparison.h"
//#include "ComparisonEngine.h"

#include "AbstractDBFile.h"
#include "Heap.h"
#include "Sort.h"
#include <string>
#include <stdarg.h>
#include <fstream>
#include <iostream>
Sort::Sort () {
    
}
Sort::~Sort () {
    
}

int Sort::Create (const char *f_path, fType f_type, void *startup) {
    //open the file with zero
    refFile->Open(0, (char *)f_path);
    //typcast startup
    SortInfo *sortInfo = (SortInfo *)startup;
    //Setting up variables for Sort instance
    order = sortInfo->myOrder;
    runLength = sortInfo->runLength; 
    type = f_type;
    return 1;
}

void Sort::Load (Schema &f_schema, const char *loadpath) {
  //
    FILE *table = fopen((char *)loadpath, "r");
    //Check if file is empty
    if (table == NULL)
    {
        //throw error
        std::cerr << "File has no records";
        //exit(1);
    }
    else
    {
        //else read the record and add to page
        Record temp;
        while (temp.SuckNextRecord(&f_schema, table) == 1)
        {
            //Add will automatically assign which page the record gets addedto
            Add(temp);
        }

        //Add would always leave the last page records in memory
        //Write those records to another page.
        //Dumping the memory contents
        //TODO: fix this
        off_t currLen = refFile->GetLength();
        off_t findPage = (currLen == 0) ? 0 : currLen - 1;
        refFile->AddPage(wBuffer, findPage);
    }
    //FIXME: DEBUG
}
int Sort::Open (const char *f_path) {
    //Open a metadata stream based on the path passed downw
    ifstream in;
    string metaFile(f_path);
    metaFile += ".METAINF";
    in.open(metaFile.c_str(),ifstream::in);

    string line;
    int tempType;
	if (in.is_open()) {
	    in >> tempType; //"sorted"
	    in >> runLength;// runLength
        order->GetFromFile(in); //setting up ordermaker
    }
    type=static_cast<fType>(tempType);
    mode = READ;


}

void Sort::MoveFirst () {
  if (mode == WRITE) {
		toggleRW();
	} else if (mode == READ) {
		currPageInd = 0;	
		//return sortedheapfile->MoveFirst();
	}
}

int Sort::Close () {
    if(mode == WRITE) toggleRW();
    return (refFile->Close() < 0) ? 0 : 1;
}

void Sort::Add (Record &rec) {
	if(mode == READ) {
		toggleRW();
	}
	iPipe->Insert(&rec);
}

int Sort::GetNext (Record &fetchme) {
off_t heapLength = refFile->GetLength();
    if(currPageInd == 0){
        //did not start to read before
        if(heapLength > 0){
            currPageInd++;
            rBuffer->EmptyItOut();
            refFile->GetPage(rBuffer,currPageInd - 1);
            rBuffer->MoveToStart();
        }
        else{
            //have nothing in file and wBuffer may contain some data
            if(wBuffer->getNumRecs() > 0 ){
                //wBuffer has some records - write it to file and read from there
                refFile->AddPage(wBuffer,currPageInd);
                wBuffer->EmptyItOut();
                rBuffer->EmptyItOut();
                refFile->GetPage(rBuffer,currPageInd);
                //make sure first record is boing pointed to
                rBuffer->MoveToStart();
                currPageInd++;
            }
            else{
                //have nothing to read
                cout<<"no record to read";
            }
        }

    }
    //current page is loaded to rBuffer
    int returnVal = rBuffer->getRecord(&fetchme);

    //if 1 end next here or else read next page to rBuffer
    heapLength = refFile->GetLength();
    if(!returnVal){
        if(currPageInd == heapLength - 1 && wBuffer->getNumRecs() == 0 ){
            //ran out of pages to read in memory 
            cout<<"Ran out of records to read"<<endl;
        }
        else if(currPageInd == heapLength - 1 && wBuffer->getNumRecs() != 0){
            //have some records in wBuffer
            refFile->AddPage(wBuffer,currPageInd);
            wBuffer->EmptyItOut();
            rBuffer->EmptyItOut();
            refFile->GetPage(rBuffer,currPageInd);
            rBuffer->MoveToStart();
            returnVal = rBuffer->getRecord(&fetchme);
            if(returnVal == 0){
                cout<<"should not ever come here . Return value should not be zero"<<endl;
            }
            currPageInd++;
        }
        else if(currPageInd < heapLength - 1){
            rBuffer->EmptyItOut();
            refFile->GetPage(rBuffer,currPageInd);
            rBuffer->MoveToStart();
            returnVal = rBuffer->getRecord(&fetchme);
            if(returnVal == 0){
                cout<<"should not ever come here. Return value should not be zero . some problem in getrecord"<<endl;
            }
            currPageInd++;

        }
        else{
            cout<<"should not ever come here";
        }

    }
    return returnVal;
}

int Sort::GetNext (Record &fetchme, CNF &cnf, Record &literal) {

}

void Sort::toggleRW(){
if(mode == READ) {
		//cout<<"in reading"<<endl;
		mode = WRITE;
		iPipe = new  (std::nothrow) Pipe(PIPE_BUFFER); 
		oPipe = new (std::nothrow) Pipe(PIPE_BUFFER);
		util = new bigq_util(); 
		util->in=iPipe;
		util->out=oPipe;
		util->sort_order=order;
		util->run_len=runLength;
		pthread_create (&thread1, NULL,run_q, (void*)util);
	}
	else if(mode == WRITE)  {
		//cout<<"in writing"<<endl;
		mode = READ;
		iPipe->ShutDown();
		//Merge_with_q();
		Merge();
        delete util;
        delVar(iPipe);
        delVar(oPipe);
		
		
	}
}
