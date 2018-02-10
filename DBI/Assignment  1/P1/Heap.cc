#include <iostream>
#include "TwoWayList.h"
#include "Record.h"
#include "Schema.h"
#include "File.h"
#include "Comparison.h"
#include "ComparisonEngine.h"
#include "DBFile.h"
#include "AbstractDBFile.h"
//#include "Defs.h"
#include "Heap.h"

Heap::Heap()
{
    //SET PAGE TO 0
    //CREATE NEW WBUFFER, RBUFFER AND REFFILE OBJECTS
    currPageInd = 0;
    wBuffer = new Page();
    rBuffer = new Page();
    refFile = new File();
    
}

int Heap::Create(const char *f_path, fType file_type,void *startup)
{
    //JUST RUN THE HEAP->OPEN WITH ZERO FLAG ON REFFILE
    refFile->Open(0, (char *)f_path);
    return 1;
}

void Heap::Load(Schema &f_schema, const char *loadpath)
{
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
        off_t currLen = refFile->GetLength();
        off_t findPage = (currLen == 0) ? 0 : currLen - 1;
        refFile->AddPage(wBuffer, findPage);
    }
    //FIXME: DEBUG
}

int Heap::Open(const char *f_path)
{
    refFile->Open(1, (char *)f_path);
    return 1;
}

void Heap::MoveFirst()
{
    currPageInd = 0;
}

int Heap::Close()
{
    return (refFile->Close() < 0) ? 0 : 1;
}

void Heap::Add(Record &rec)
{
    //Check if buffer can be written to existing page
    if (wBuffer->Append(&rec) == 0)
    {
        //If not, dump the existing page
        off_t currLen = refFile->GetLength();
        off_t findPage = currLen == 0 ? 0 : currLen - 1;
        refFile->AddPage(wBuffer, findPage);
        //Now delete the wBuffer
        wBuffer->EmptyItOut();
        //Append current records to the wBuffer
        wBuffer->Append(&rec);
    }
}
int Heap::GetNext(Record &fetchme)
{
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

int Heap::GetNext(Record &fetchme, CNF &cnf, Record &literal)
{
    bool compFlag = false;
    ComparisonEngine engine;
    while (!compFlag)
    {
        if (GetNext(fetchme) == 1)
        {
            compFlag = (bool)engine.Compare(&fetchme, &literal, &cnf);
        }
        else
            return 0; //FIXME: try return false;
    }
    return (int)compFlag; //FIXME: try return compFlag;
}
Heap::~Heap()
{
    //DESTROY THE OBJECT CREATE ABOVE
    delete wBuffer;
    delete rBuffer;
    delete refFile;
}
