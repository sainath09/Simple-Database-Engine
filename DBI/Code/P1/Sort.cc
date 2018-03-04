
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
  heapDB = new Heap();
    order = new OrderMaker();
     buildNewQuery=false;
    queryBuildable = true;
	
}
Sort::~Sort () {
    delete heapDB;
    delete order;
}
void* startDifferentialFile (void *arg) {	
	structBigQ *temp = (structBigQ *) arg;
	BigQ b_queue(*(temp->iPipe),*(temp->oPipe),*(temp->order),temp->runlen);
}

int Sort::Create (const char *f_path, fType f_type, void *startup) {
    //open the file with zero
    heapDB->Create(f_path,heap,NULL);
    //typcast startup
    SortInfo *sortInfo = (SortInfo *)startup;
    //Setting up variables for Sort instance
    order = sortInfo->myOrder;
    runLength = sortInfo->runLength; 
    type = f_type;
    //Needed for the last step of merge
    fpath = f_path;
    
    return 1;
}

void Sort::Load (Schema &f_schema, const char *loadpath) {
  //
    FILE *readFile = fopen((char *)loadpath, "r");
    //Check if file is empty
    if (readFile == NULL)
    {
        //throw error
        std::cerr << "File has no records";
    }
    else
    {
        //else read the record and add to page
        Record temp;
        while (temp.SuckNextRecord(&f_schema, readFile) == 1)
        {
            //Add the record to iPipe
            Add(temp);
        }
    }
    
}

int Sort::Open (const char *f_path) {
    //Open a metadata stream based on the path passed downw
    ifstream inFStream;
    string metaFile(f_path);
    metaFile += ".METAINF";
    inFStream.open(metaFile.c_str(),ifstream::in);

    string line;
    string tempType;
	if (inFStream.is_open()) {
	    getline(inFStream,tempType); //"sorted"
	    inFStream >> runLength;// runLength
        order->GetFromFile(inFStream); //setting up ordermaker
    }
    if(tempType=="sorted")
        type=sorted;
    else 
        cerr<<"Weird data in .metainf file, file line should read sorted.";
 

    rwmode = READ;
    fpath = f_path;
    //now open the dumb file to the fpath location too.
    return heapDB->Open(f_path);

}

void Sort::MoveFirst () {
    buildNewQuery=false;
    queryBuildable = true;
  if (rwmode == WRITE) toggleRW();

    heapDB->MoveFirst();
}

int Sort::Close () {
    if(rwmode == WRITE) toggleRW();
    return heapDB->Close();
}

void Sort::Add (Record &rec) {
	if(rwmode == READ) toggleRW();

	iPipe->Insert(&rec);
}

int Sort::GetNext (Record &fetchme) {
    if (rwmode == WRITE) toggleRW();
	
    return heapDB->GetNext(fetchme);  
}

int Sort::GetNext (Record &fetchme, CNF &cnf, Record &literal) {
    if (rwmode == WRITE) {
        toggleRW();
        buildNewQuery=false;
		queryBuildable=true; 
    }

    if (queryBuildable && buildNewQuery){
        GetNextQuery(fetchme, cnf, literal);
    }
    else if (queryBuildable && !buildNewQuery){
        buildNewQuery = true;
        int search;
        query=new OrderMaker;
        if(cnf.genQOrder(*query,*order)>0)
        {  
            search=binarySearch(fetchme,cnf,literal);
            ComparisonEngine engine;
            if(search)
            {
                if (engine.Compare (&fetchme, &literal, &cnf))          
                    return 1;
                else
                {
                    GetNextQuery(fetchme, cnf, literal);
                }
            }
            else
                return 0;
        }else{
            queryBuildable=false;
			return heapDB->GetNext(fetchme, cnf, literal);	
        }
    }
    else if (!queryBuildable)
        return heapDB->GetNext(fetchme, cnf, literal);	
  return 0;  // no matching records
}
bool Sort::binarySearch(Record& fetchMe,CNF &cnf,Record& literal){
    off_t start = heapDB->getPageIndex();
    off_t last = heapDB->getLength()-2;
    Page* midP = new Page();
    bool recFound = false;
    ComparisonEngine ce;
    off_t mid;
    while(start<last){
        mid = (start+last)/2;
        heapDB->getPage(midP,mid);
        if(midP->getRecord(&fetchMe)){ //FIXME: get record instead of get first
            if(ce.Compare (&literal, query, &fetchMe,order) <= 0) last = mid;
            else{
                start = mid;
                if(start == last -1){
                    heapDB->getPage(midP,last);
                    midP->getRecord(&fetchMe); //FIXME: get record instead of get first
                    if (ce.Compare (&literal, query, &fetchMe,order) > 0) mid=last;
					break;
                }
            }
        }
        else break;
    }
    //our record that is needed is in mid page 
    heapDB->getPage(pageBuffer,mid);
    while(pageBuffer->getRecord(&fetchMe)){//FIXME: get record instead of get first
        if(ce.Compare (&literal, query, &fetchMe,order) == 0 ){
            recFound=true;
            heapDB->setPageIndex(mid);
            break;
        }
    }
    //corner case of having just 2 pages and our record is first of second page
    if(!recFound && mid < heapDB->getLength()-2){
        heapDB->getPage(pageBuffer,mid+1);
        if(pageBuffer->GetFirst(&fetchMe) && ce.Compare (&literal, query, &fetchMe,order) == 0){
            recFound=true;
            heapDB->setPageIndex(mid+1);
        }
    }
    return recFound;
}
int Sort :: GetNextQuery(Record &fetchme, CNF &cnf, Record &literal)
{ 
    ComparisonEngine engine;
    while(true){
        if(pageBuffer->getRecord(&fetchme)) //FIXME: changed from getFirst to getRecord
        {         
            if(engine.Compare (&literal, query, &fetchme,order) ==0){
                if (engine.Compare (&fetchme, &literal, &cnf)) return 1;
            }
            else return 0;
        }
        else{
            int nextPage =heapDB->getPageIndex()+1;
            heapDB->setPageIndex(nextPage);
            if(nextPage<heapDB->getLength()-1) 
                heapDB->getPage(pageBuffer,nextPage);          
            else              
                return 0;
        }
    }
}

void Sort::toggleRW(){
     if(rwmode == WRITE)  {
		rwmode = READ;
		iPipe->ShutDown();
		mergeDB();
        delete stBigQ;
        delVar(iPipe);
        delVar(oPipe);
		
		
	}else if(rwmode == READ) {

		rwmode = WRITE; 
		iPipe = new Pipe(PIPE_BUFFER); 
		oPipe = new Pipe(PIPE_BUFFER);
		stBigQ = new structBigQ(); 
		stBigQ->iPipe=iPipe;
		stBigQ->oPipe=oPipe;
		stBigQ->order=order;
		stBigQ->runlen=runLength; 
		pthread_create (&diffrentialThread, NULL, startDifferentialFile, (void*)stBigQ);
	}
	
}
void Sort::mergeDB(){
    iPipe->ShutDown();
    Record file,out;
    ComparisonEngine cmp;
    bool flagO = false;
    flagO=oPipe->Remove(&out);
    bool flagF=false;
    Heap * resultHeap;
    
    //Create a dump file
    
    char *rFileName = "fbvjd.cadx";

    resultHeap->Create(rFileName, heap, NULL);

    if(heapDB->isEmpty()&&flagO){
        //Add that first record in there
        heapDB->Add(out);
		while(oPipe->Remove(&out)) {
			  heapDB->Add(out);
		}
        //write the last records from wbuffer to the Heaptoo
       heapDB->wBuffertoPage();
		
    }
    else if(!flagO){
        //helps cut down computation
    }
    else{
        heapDB->MoveFirst();
        flagF = heapDB->GetNext(file);
        //already have value in out
        //Implement simple 2 way merge.
        while (flagF || flagO){
            if (!flagF || (flagO && cmp.Compare(&file, &out, order) > 0)) {
                resultHeap->Add(out);
                flagO = oPipe->Remove(&out);
            } else if (!flagO || (flagF && cmp.Compare(&file, &out, order) <= 0)) {
                resultHeap->Add(file);
                flagF = GetNext(file);
            } 

        }//end of while
        delete heapDB;
		//Rename tmp fill
		rename(rFileName,fpath);
		heapDB = resultHeap;

    }//end of else

}
