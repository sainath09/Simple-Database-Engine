#include "BigQ.h"


//From stack overflow to generate random string in C++11
// std::string random_string( size_t length )
// {
//     auto randchar = []() -> char
//     {
//         const char charset[] =
//         "0123456789"
//         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//         "abcdefghijklmnopqrstuvwxyz";
//         const size_t max_index = (sizeof(charset) - 1);
//         return charset[ rand() % max_index ];
//     };
//     std::string str(length,0);
//     std::generate_n( str.begin(), length, randchar );
//     return str;
// }

OrderMaker *orderMaker;

static bool sortOrder(Record* r1,Record* r2){
	ComparisonEngine comp;
	if(comp.Compare(r1,r2,orderMaker)<0) return true;
	else return false;
}

class sortMergeOrder{

public:
	sortMergeOrder(){}

	bool operator()(Record* r1,Record* r2) {
		ComparisonEngine comp;
		if(comp.Compare(r1,r2,orderMaker)<0) return false;
		else return true;
	}
};



BigQ :: BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen) {
	//Check if there is data to be read otherwise shutdown
	if(runlen<=0){
		out.ShutDown();
		return;
	}
	//initalizing global orderMaker
	orderMaker = &sortorder;
	//map for the overflow values
	map<int,Page *> overflow;

	int totalPagesinRun=0; //TODO: count
	int pageNumber=0; //TODO: page_count

	Record readRecord; //TODO: rec
	

	Page* tempPage = new Page(); // TODO: p
	vector<Record*> rVector; //TODO: rec_vector
	vector<Page*> pVector;//TODO: page_vector

	File oFile; //TODO: f
	// string metaFile(random_string(12)); 
	// metaFile+="asdadasdads.dat";
	string metaFile="aDASsdadasd312SD3121ads.dat";
	oFile.Open(0, (char *)metaFile.c_str()); //FIXME: Maybe a more elegant way exists to do this, random string maybe

	while(1){
		// read data from in pipe sort them into runlen pages
		if(in.Remove(&readRecord) && totalPagesinRun<runlen) {
			//now 
			bool pageNotFull = tempPage->Append(&readRecord);
			if(!pageNotFull) {
				//Add current page to page vector if page is full
				pVector.push_back(tempPage);
				//clean the temp page
				tempPage = new Page();
				//write new records to this temp page now
				tempPage->Append(&readRecord);
				//Increment number of pages included in this 1 run
				totalPagesinRun++;
			}
			else { 
				//In case all reads are over and they didnt fill a single page, 
				//still add that incomplete page to vector
				if(totalPagesinRun<runlen && tempPage->getNumRecs()) {
					pVector.push_back(tempPage);
				}
				//TODO: Figure out if we are missing any source of information to push into vector
				Record * readPVec;//Create a temp Record to dump values from pVector

				//Now read all the pages in pVector and sort and put them in rVector
				for(int i=0;i<pVector.size();i++) {
					readPVec = new Record();
					while(pVector[i]->GetFirst(readPVec)) {
						rVector.push_back(readPVec);
						readPVec = new Record();
					}
					// Just dumping out values in correct garbage collection manner
					delVar(readPVec);
					delVar(pVector[i]);  
				}
				//Using standard sorting algo to sort all the records in the run
				std::sort(rVector.begin(),rVector.end(),sortOrder);

				//Write the sorted record into pages
				int pagesWritten=0; //TODO: page_count1
				Page* wPage =new Page(); //TODO: file_page // temp page to write records to file 
				for(int i=0;i<rVector.size();i++) {
					//pageNotFull is also defined above in a different scope, just reusing that variable name here
					bool pageNotFull = wPage->Append(rVector[i]);
					if(!pageNotFull) {
						pagesWritten++;
						oFile.AddPage(wPage,pageNumber++);
						wPage->EmptyItOut();
						wPage->Append(rVector[i]);
					}
					// Just dumping out values in correct garbage collection manner
					delVar(rVector[i]);  
				}
				//now if there are some records in the end, write all of them in the last page of file

				if(pagesWritten<runlen) {
					//Include records which lie inside run length of a partial page read
					//Only run if there are any actual records to read
					if(wPage->getNumRecs()){
							pagesWritten++;
							oFile.AddPage(wPage,pageNumber++);
					}
				}
				else { 
					//Exlude records which lie outside run length, but have read partials pages into the run and send them to an overflow
					//Only run if there are any actual records to read
					if(wPage->getNumRecs()) {	
						overflow[pageNumber-1]=wPage;
					}
				}
				wPage=NULL;  
				rVector.clear();
				pVector.clear();

				//Checking the status of pages to figure out if next run is going to start 
				if (totalPagesinRun>=runlen) {
					//Already have read the first record for next run if possible, append it here in available
					tempPage->Append(&readRecord);
					//Reset the number of pages in run, so that we can sort then next runlen records
					totalPagesinRun=0;
					continue;
				}
				else {
					break;
				}

			}
		}

	} //End of while

	// construct priority queue over sorted runs and dump sorted data into the out pipe
	priority_queue<Record*,vector<Record*>,sortMergeOrder> pQueue;

	map<Record *,int> mapRecordtoRun; //TODO: m_record
	off_t pagesInFile=oFile.GetLength(); //TODO: file_length
	off_t totalRuns; //TODO: no_of_runs
		if(pagesInFile==0) totalRuns=0;
		else
			totalRuns=((ceil)((float)(pagesInFile-1)/runlen));

	off_t pagesinLastRun=(pagesInFile-1)-(totalRuns-1)*runlen; //TODO: last_run_pages
	//Find out pages in each run
	vector<int> pagesInRun(totalRuns); //TODO: page_index

	//Buffer for Page inside a run
	vector<Page*> tempPageInRun(totalRuns); //TODO: page_array

	//index value of current page in a run
	int pIndex=0; //TODO: page_num

	for(int i=0;i<totalRuns;i++) {
		tempPageInRun[i]=new Page();
		//Get the first records of first page in the run
		oFile.GetPage(tempPageInRun[i],pIndex);
		//Set this value to say at least one page is in this run
		pagesInRun[i]=1;
		Record * readRecFromIndex = new Record();
		tempPageInRun[i]->GetFirst(readRecFromIndex);
		//Push first record from each run into the priority queue
		pQueue.push(readRecFromIndex);
		//rec_head[i]=r;
		mapRecordtoRun[readRecFromIndex]=i;
		readRecFromIndex=NULL;
		//Jump to next run and read the first record from that one too
		pIndex+=runlen;
	}

	
	while(!pQueue.empty()) {
		//push out the (hopefully) minimum record from the priority queue
		Record* minRecFromQ=pQueue.top(); //TODO: r
		//delete it from the queue
		pQueue.pop();

		int recBelongsinRun=-10; // TODO: next_rec_run
		//Now try to find the popped record in the map, it should return which run does that record belong to
		recBelongsinRun=mapRecordtoRun[minRecFromQ];
		//Delete this record from map and delete the referenced  address too to remove any confusion
		//No need to run another delete on the buffer, its already removed from there
		mapRecordtoRun.erase(minRecFromQ);

		if(recBelongsinRun == -10) {
			cout<<"Record not mapped correctly to any run."<<endl;
			return;
		}

		Record* findNextRecordinRun = new Record(); //TODO: next
		bool foundRec=true; //TODO: is_rec_found
		if(!tempPageInRun[recBelongsinRun]->GetFirst(findNextRecordinRun)) {
			//Making sure we are not on last page
 			if((!(recBelongsinRun==(totalRuns-1)) && pagesInRun[recBelongsinRun]<runlen) || ((recBelongsinRun==(totalRuns-1) && pagesInRun[recBelongsinRun]<pagesinLastRun))){
				
				oFile.GetPage(tempPageInRun[recBelongsinRun],pagesInRun[recBelongsinRun]+recBelongsinRun*runlen);
				tempPageInRun[recBelongsinRun]->GetFirst(findNextRecordinRun);
				tempPageInRun[recBelongsinRun]++;
			}
			else {  
				if(pagesInRun[recBelongsinRun]==runlen)
				 {
					if(overflow[(recBelongsinRun+1)*runlen-1]) {
						delete tempPageInRun[recBelongsinRun];
						tempPageInRun[recBelongsinRun]=NULL;
						tempPageInRun[recBelongsinRun]=(overflow[(recBelongsinRun+1)*runlen-1]);
						overflow[(recBelongsinRun+1)*runlen-1]=NULL;
						tempPageInRun[recBelongsinRun]->GetFirst(findNextRecordinRun);
					}
					else foundRec=false;
				}
				else foundRec=false;
			}
		}
		
		//Push the next record into the priority queue if found 
		if(foundRec) {
			pQueue.push(findNextRecordinRun);
		}
		mapRecordtoRun[findNextRecordinRun]=recBelongsinRun;
		//rec_head[next_rec_run]=next;


		out.Insert(minRecFromQ); //Put the min priority record onto the pipe
		//cout<<temp_file<<endl;
	}

	oFile.Close(); 
	remove((char *)metaFile.c_str()); 
		
    // finally shut down the out pipe
	out.ShutDown();
}

BigQ::~BigQ () {
}
