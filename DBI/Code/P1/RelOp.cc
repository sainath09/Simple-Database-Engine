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


void* joinFunc(void * args){
	stProject* stprj = (stProject*) args;
	stJoin *stj = (stJoin*) args;
	Pipe * inPipeL=stj->inPipeL ;
	Pipe * inPipeR=stj->inPipeR ;
	Pipe * outPipe=stj->outPipe ;
	CNF *selOp=stj->selOp ;
	Record *literal=stj->literal ;
	
	OrderMaker omL,omR;
	Pipe *bqL = new Pipe(PIPE_BUFFER);
	Pipe *bqR = new Pipe(PIPE_BUFFER);
	Record tempR,tempL;
	ComparisonEngine ce;

	int commonAtts = selOp->GetSortOrders(omL,omR);

	if(commonAtts<1){
		//Block Nested as there are no common attributes


	}else{
		//try to find using points of commonality

		//Manually defined runlength as 10 here
		BigQ bigqleft(*inPipeL, *bqL, omL, 10);
		BigQ bigqright(*inPipeR, *bqR, omR, 10);

		bqL->Remove(&tempL);
		bqR->Remove(&tempR);

		bool FLAG_L, FLAG_R = false;
		int int_l = (&tempL->bits)[1] / sizeof(int) -1; 
		int int_r = (&tempR->bits)[1] / sizeof(int) -1;



	}


}

void Join::Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal){
	stJoin *stj = new stJoin();
	stj->inPipeL = &inPipeL;
	stj->inPipeR = &inPipeR;
	stj->outPipe = &outPipe;
	stj->selOp = &selOp;
	stj->literal = &literal;
	//stj->RunPages = RunPages>0?RunPages:1;
	pthread_create (&thread, NULL, joinFunc, (void *)stj);
}

void Join::WaitUntilDone () {
	 pthread_join (thread, NULL);
}

void Join::Use_n_Pages (int runlen) {

}



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
void* sumFunc(void* args){
	stSum* stsum = (stSum *)args;
	Pipe* inPipe = stsum->inPipe;
	Pipe* outPipe = stsum->outPipe;
	Function* computeMe = stsum->computeMe;
	int temp_int = 0;
	int sumInt = 0;
	double temp_double = 0.0;
	double sumDouble = 0.0;
	Record tempRec;
	Attribute attr;
	bool FLAG = true;
	attr.name = (char *)"sum";
	while(inPipe->Remove(&tempRec)){
		Type t = computeMe->Apply(tempRec,temp_int,temp_double);
		if(FLAG){
			if(t == Int) attr.myType = Int;
			else if(t == Double) attr.myType = Double;
			FLAG  = false;
		}
		if(t == Double){
			sumDouble += temp_double;
		}
		else if( t == Int){
			sumInt += temp_int;			
		}
		else { 
			cerr<<"Invalid type return from computeME->apply encountered in Sum thread func "<<endl;
		}
	}
	char s[15];
	if(attr.myType == Int){
		sprintf(s,"%d|",sumInt);
	}
	else if(attr.myType == Double){
		sprintf(s,"%f|",sumDouble);
	}
	else { 
		cerr<<"Invalid type return from computeME->apply encountered in Sum thread func "<<endl;
	}
	Schema tempSch("schsum", 1, &attr);
	tempRec.ComposeRecord(&tempSch,(const char *)&s[0]);
	outPipe->Insert(&tempRec);
	outPipe->ShutDown();
	delVar(attr.name); //FIXME: check for error here.

}
void Sum::Run (Pipe &inPipe, Pipe &outPipe, Function &computeMe){
	stSum* stsum = new stSum();
	stsum->inPipe = &inPipe;
	stsum->outPipe = &outPipe;
	stsum->computeMe = &computeMe;
	pthread_create(&thread,NULL,sumFunc,(void *)stsum);
}

void Sum::WaitUntilDone () {
	 pthread_join (thread, NULL);
}

void Sum::Use_n_Pages (int runlen) {

}
void * grpbyFunc(void * args){
	stGroupBy* stgby = (stGroupBy*)args;
	Pipe* inPipe = stgby->inPipe;
	Pipe* outPipe = stgby->outPipe;
	OrderMaker* groupAtts = stgby->groupAtts;
	Function* computeMe = stgby->computeMe;
	int attArray[MAX_ANDS];
	groupAtts->getAttrs(attArray);
	int gbyNumAtts = groupAtts->getNumAtts();
	int* gbyAList = new int[MAX_ANDS + 1];

	for(int i = 0;i<gbyNumAtts  +1;i++){
		if(i == 0) gbyAList[i] = 0;
		else gbyAList[i] = attArray[i-1];
	}
	
	Record tempRec;
	Record tempRec2;
	Attribute attr;
	int temp_int = 0;
	int sumInt = 0;
	double temp_double = 0.0;
	double sumDouble = 0.0;
	attr.name = "sum";
	bool FLAG = true;
	int currNumAtts = 0;
	char s[100];
	ComparisonEngine ce;
	Schema tempsch("sum", 1, &attr);
	while(inPipe->Remove(&tempRec)){
		Type t = computeMe->Apply(tempRec,temp_int,temp_double);
		if(FLAG){
			if(t == Int) {
				attr.myType = Int;
				sumInt += temp_int;	
			}
			else if(t == Double) {
				attr.myType = Double;
				sumDouble += temp_double;
			}
			FLAG  = false;
			currNumAtts = tempRec.getNumAtts();
			//tempRec2.Consume(&tempRec);
		}
		else{
			if(!ce.Compare(&tempRec2,&tempRec,groupAtts)){
				if(t == Double){
				sumDouble += temp_double;
				}
				else if( t == Int){
					sumInt += temp_int;			
				}
				else { 
					cerr<<"Invalid type return from computeME->apply encountered in group by thread func "<<endl;
				}

			}
			else{
				
				if(attr.myType == Int){
					sprintf(s,"%d|",sumInt);
					sumInt = temp_int;
				}
				else if(attr.myType == Double){
					sprintf(s,"%f|",sumDouble);
					sumDouble = temp_double;
				}
				else { 
					cerr<<"Invalid type return from computeME->apply encountered in group by thread func "<<endl;
				}
				//FIXME: s should be cleared before this.
				Record sumRec;
				Record resRec;
				sumRec.ComposeRecord(&tempsch,(const char *) &s[0]);
				tempRec2.Project (attArray, gbyNumAtts, currNumAtts);
				resRec.MergeRecords (&sumRec, &tempRec2, 1, gbyNumAtts, gbyAList, gbyNumAtts+1, 1);
				outPipe->Insert(&resRec);

			}
		}
		tempRec2.Consume(&tempRec);
	}

	if(attr.myType == Int) sprintf(s,"%d|",sumInt);
	else if(attr.myType == Double) sprintf(s,"%f|",sumDouble);
	else { 
		cerr<<"Invalid type return from computeME->apply encountered in group by thread func "<<endl;
	}
	//FIXME: s should be cleared before this.
	Record sumRec;
	Record resRec;
	sumRec.ComposeRecord(&tempsch,(const char *) &s[0]);
	tempRec2.Project (attArray, gbyNumAtts, currNumAtts);
	resRec.MergeRecords (&sumRec, &tempRec2, 1, gbyNumAtts, gbyAList, gbyNumAtts+1, 1);
	outPipe->Insert(&resRec);
	outPipe->ShutDown();
	delVar(attr.name);
	delete []gbyAList;
}

void GroupBy::Run (Pipe &inPipe, Pipe &outPipe, OrderMaker &groupAtts, Function &computeMe){
	stGroupBy* stgby = new stGroupBy();
	stgby->inPipe = &inPipe;
	stgby->outPipe = &outPipe;
	stgby->groupAtts = &groupAtts;
	stgby->computeMe = &computeMe;
	pthread_create(&thread,NULL,grpbyFunc,(void*)stgby);
}

void GroupBy::WaitUntilDone () {
	 pthread_join (thread, NULL);
}

void GroupBy::Use_n_Pages (int runlen) {

}



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



