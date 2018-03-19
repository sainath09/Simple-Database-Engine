#include "RelOp.h"

void* SelFileFunc(void* args){
	Record tempRec;
	ComparisonEngine ce;
	stSelFile* stSFile = (stSelFile *)args;
	DBFile* inFile = stSFile->inFile;
	Pipe* outPipe = stSFile->outPipe;
	CNF* selOp = stSFile->selOp;
	Record* literal = stSFile->literal;
	while(inFile->GetNext(tempRec)){
		if(selOp == NULL || ce.Compare(&tempRec,literal,selOp)){
			outPipe->Insert(&tempRec);
		}
	}
	outPipe->ShutDown();
}

void SelectFile::Run (DBFile &inFile, Pipe &outPipe, CNF &selOp, Record &literal) {
	stSelFile* stSFile = new stSelFile();
	stSFile->inFile = &inFile;
	stSFile->outPipe = &outPipe;
	stSFile->selOp = &selOp;
	stSFile->literal = &literal;
	pthread_create(&thread,NULL,SelFileFunc,(void *)stSFile);
}

void SelectFile::WaitUntilDone () {
	 pthread_join (thread, NULL);
}

void SelectFile::Use_n_Pages (int runlen) {

}


void* SelPipeFunc(void* args){
	Record tempRec;
	ComparisonEngine ce;
	stSelPipe* stSPipe = (stSelPipe *)args;
	Pipe* inPipe = stSPipe->inPipe;
	Pipe* outPipe = stSPipe->outPipe;
	CNF* selOp = stSPipe->selOp;
	Record* literal = stSPipe->literal;
	while(inPipe->Remove(&tempRec)){
		if(selOp == NULL || ce.Compare(&tempRec,literal,selOp)){
			outPipe->Insert(&tempRec);
		}
	}
	outPipe->ShutDown();
}


void SelectPipe::Run(Pipe &inPipe, Pipe &outPipe, CNF &selOp, Record &literal) {
	stSelPipe* stSPipe = new stSelPipe();
	stSPipe->inPipe = &inPipe;
	stSPipe->outPipe = &outPipe;
	stSPipe->selOp = &selOp;
	stSPipe->literal = &literal;
	pthread_create(&thread,NULL,SelPipeFunc,(void *)stSPipe);

}
void SelectPipe::WaitUntilDone () {
	 pthread_join (thread, NULL);
}

void SelectPipe::Use_n_Pages (int runlen) {

}

void * projFunc(void* args){
	Record tempRec;
	ComparisonEngine ce;
	stProject* stprj = (stProject*) args;
	Pipe* inPipe = stprj->inPipe;
	Pipe* outPipe = stprj->outPipe;
	int* keepMe = stprj->keepMe;
	int numAttsInput = stprj->numAttsInput;
	int numAttsOutput = stprj->numAttsOutput;
	while(inPipe->Remove(&tempRec)){
		tempRec.Project(keepMe,numAttsOutput,numAttsInput);
		outPipe->Insert(&tempRec);
	}
	outPipe->ShutDown();
	
}
void Project::Run (Pipe &inPipe, Pipe &outPipe, int *keepMe, int numAttsInput, int numAttsOutput) {
	stProject* stprj = new stProject();
	stprj->inPipe = &inPipe;
	stprj->outPipe = &outPipe;
	stprj->keepMe = keepMe;
	stprj->numAttsInput = numAttsInput;
	stprj->numAttsOutput = numAttsOutput;
	pthread_create(&thread,NULL,projFunc,(void *)stprj);
}

void Project::WaitUntilDone () {
	 pthread_join (thread, NULL);
}

void Project::Use_n_Pages (int runlen) {

}

// void Join::Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal){

// }

// void Join::WaitUntilDone () {
// 	 pthread_join (thread, NULL);
// }

// void Join::Use_n_Pages (int runlen) {

// }
void *runBigqFunc(void * args){
	structBigQ *t = (structBigQ *) args;
	BigQ queue(*(t->iPipe),*(t->oPipe),*(t->order),t->runlen);
} 
void* dupRemFunc(void* args){
	stDupRem* stduprem = (stDupRem*) args;
	Pipe* inPipe = stduprem->inPipe;
	Pipe* outPipe = stduprem->outPipe;
	Schema* mySchema = stduprem->mySchema;
	ComparisonEngine ce;
	Record tempRec;
	Record nextRec;
	OrderMaker om(mySchema);
	Pipe* tempIpPipe = new Pipe(PIPE_BUFFER);
	Pipe* tempOpPipe = new Pipe(PIPE_BUFFER);
	structBigQ* sbq = new structBigQ();
	sbq->iPipe = tempIpPipe;
	sbq->oPipe = tempOpPipe;
	sbq->order = &om;
	sbq->runlen = 2;
	pthread_t tempThread;
	pthread_create(&tempThread,NULL,runBigqFunc,(void *) sbq);
	while(inPipe->Remove(&tempRec)){
		tempIpPipe->Insert(&tempRec);
	}
	tempIpPipe->ShutDown();
	bool FLAG_FIRST = true;
	while(tempOpPipe->Remove(&nextRec)){
		if(FLAG_FIRST){
			tempRec.Copy(&nextRec);
			FLAG_FIRST = false;
		}
		else{
			if(!ce.Compare(&nextRec,&tempRec,&om)){

			}
			else {
				outPipe->Insert(&tempRec);
				tempRec.Copy(&nextRec);
			}
		}

	}
	outPipe->ShutDown();
	pthread_join(tempThread,NULL);
	tempOpPipe->ShutDown();
}
void DuplicateRemoval::Run (Pipe &inPipe, Pipe &outPipe, Schema &mySchema){
	stDupRem* stduprem = new stDupRem();
	stduprem->inPipe = &inPipe;
	stduprem->outPipe = &outPipe;
	stduprem->mySchema = &mySchema;
	pthread_create(&thread,NULL,dupRemFunc,(void *)stduprem);


}

void DuplicateRemoval::WaitUntilDone () {
	 pthread_join (thread, NULL);
}

void DuplicateRemoval::Use_n_Pages (int runlen) {

}

// void Sum::Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe){
// }

// void Sum::WaitUntilDone () {
// 	 pthread_join (thread, NULL);
// }

// void Sum::Use_n_Pages (int runlen) {

// }


// void GroupBy::Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe);{

// }

// void GroupBy::WaitUntilDone () {
// 	 pthread_join (thread, NULL);
// }

// void GroupBy::Use_n_Pages (int runlen) {

// }



void* woutFunc(void* args){
	Record tempRec;
	ComparisonEngine ce;
	stWOut* stwout = (stWOut*) args;
	FILE* outFile = stwout->outFile;
	Pipe* inPipe = stwout->inPipe;
	Schema* mySchema = stwout->mySchema;
	while(inPipe->Remove(&tempRec)){
		tempRec.PrintToFile(outFile,mySchema);
	}
}

void WriteOut::Run (Pipe &inPipe, FILE *outFile, Schema &mySchema)  {
	stWOut* stwout = new stWOut();
	stwout->inPipe = &inPipe;
	stwout->outFile = outFile;
	stwout->mySchema = &mySchema;
	pthread_create(&thread,NULL,woutFunc,(void *)stwout);
}

void WriteOut::WaitUntilDone () {
	 pthread_join (thread, NULL);
}

void WriteOut::Use_n_Pages (int runlen) {

}



