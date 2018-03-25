#include "RelOp.h"

void *runBigqFunc(void * args){
	structBigQ *t = (structBigQ *) args;
	BigQ queue(*(t->iPipe),*(t->oPipe),*(t->order),t->runlen);
} 

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
	pages = runlen;

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
	pages = runlen;

}


void* joinFunc(void * args){
	stProject* stprj = (stProject*) args;
	stJoin *stj = (stJoin*) args;
	Pipe * inPipeL=stj->inPipeL ;
	Pipe * inPipeR=stj->inPipeR ;
	Pipe * outPipe=stj->outPipe ;
	CNF *selOp=stj->selOp ;
	Record *literal=stj->literal ;
	pthread_t threadLeft,threadRight;
	OrderMaker omL,omR;
	Pipe *bqL = new Pipe(PIPE_BUFFER);
	Pipe *bqR = new Pipe(PIPE_BUFFER);
	Record tempR,tempL,resRec;
	ComparisonEngine ce;
	int FLAG_L = 0;
	int FLAG_R = 0;
	int commonAtts = selOp->GetSortOrders(omL,omR);

	if(commonAtts<1){
		//Block Nested as there are no common attributes
		vector<Record *> rightRec;
		FLAG_L = inPipeL->Remove(&tempL);
		FLAG_R = inPipeR->Remove(&tempR);
		int attrL = tempL.getNumAtts();
		int attrR = tempR.getNumAtts();		
		int* resultAttr = new int[attrL + attrR];
		for(int i = 0;i<attrL;i++) resultAttr[i] = i;
		for(int j = 0;j<attrR;j++) resultAttr[attrL + j] = j;
		while(FLAG_R){
			Record temp;
			temp.Consume(&temp);
			FLAG_R = inPipeR->Remove(&tempR);
			rightRec.push_back(&temp);
		}
		while(FLAG_L){
			for(int i = 0;i<rightRec.size();i++){
				resRec.MergeRecords(&tempL,rightRec[i],attrL,attrR,resultAttr, attrL+attrR,attrL);
				outPipe->Insert(&resRec);
			}
			FLAG_L = inPipeL->Remove(&tempL);
		}
		delete[] resultAttr;
	}else{
		//try to find using points of commonality
		structBigQ* sbql = new structBigQ();
		sbql->iPipe = inPipeL;
		sbql->oPipe = bqL;
		sbql->order = &omL;
		sbql->runlen = 10; //Manually defined runlength as 10 here		
		structBigQ* sbqr = new structBigQ();
		sbqr->iPipe = inPipeR;
		sbqr->oPipe = bqR;
		sbqr->order = &omR;
		sbqr->runlen = 10; //Manually defined runlength as 10 here		
		pthread_create(&threadLeft,NULL,runBigqFunc,(void *) sbql);
		pthread_create(&threadRight,NULL,runBigqFunc,(void *) sbqr);
		FLAG_L = bqL->Remove(&tempL);
		FLAG_R = bqR->Remove(&tempR);
		int attrL = tempL.getNumAtts();
		int attrR = tempR.getNumAtts();		
		int* resultAttr = new int[attrL + attrR];
		for(int i = 0;i<attrL;i++) resultAttr[i] = i;
		for(int j = 0;j<attrR;j++) resultAttr[attrL + j] = j;
		while(FLAG_L && FLAG_R){ // FLAGS should automatically end this loop in case one pipe shutsdown
			int temp = ce.Compare(&tempL,&omL,&tempR,&omR);
			// if(temp > 0 &&!bqL->Remove(&tempL)) continue; //FIXME: corner c
			// else if(temp < 0 &&!bqR->Remove(&tempR)) continue; 
			switch(temp){
				case -1: 
					FLAG_R = bqR->Remove(&tempR);
					break;
				case 1 : 
					FLAG_L = bqL->Remove(&tempL);
					break;
				default : 
					vector<Record*> rightRecords;
					Record tempPrev;
					tempPrev.Consume(&tempR); //copying first record to tempPrev
					rightRecords.push_back(&tempPrev); //adding first record to vector list
					FLAG_R = bqR->Remove(&tempR);   //checking if next record exists 
					while( FLAG_R && ce.Compare(&tempPrev,&tempR,&omR) == 0){ //FIXME: did a b == c comparision may need to do a == b across tables
						rightRecords.push_back(&tempR); //TODO: there may be address issues may have to use tempPrev everywhere
						//tempPrev.Consume(&tempR); FIXME: not require?
						FLAG_R = bqR->Remove(&tempR); 
					}
					tempPrev.Consume(&tempL);
					FLAG_L = bqL->Remove(&tempL);
					for(int i = 0;i<rightRecords.size() && FLAG_L;i++){
						resRec.MergeRecords(&tempPrev,rightRecords[i],attrL,attrR,resultAttr,attrL + attrR,attrL);
						outPipe->Insert(&resRec);
					}
					while(FLAG_L && ce.Compare(&tempPrev,&tempL,&omL) == 0){
						for(int i = 0;i<rightRecords.size();i++){
							resRec.MergeRecords(&tempL,rightRecords[i],attrL,attrR,resultAttr,attrL + attrR,attrL);
							outPipe->Insert(&resRec);
						}
						FLAG_L = bqL->Remove(&tempL);
					}
				break;
			}

		}
		delVar(bqL);
		delVar(bqR);
		delete[] resultAttr;
		delVar(sbql);
		delVar(sbqr);
	}
	outPipe->ShutDown();


}

void Join::Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal){
	stJoin *stj = new stJoin();
	stj->inPipeL = &inPipeL;
	stj->inPipeR = &inPipeR;
	stj->outPipe = &outPipe;
	stj->selOp = &selOp;
	stj->literal = &literal;
	//stj->RunPages = RunPages>0?RunPages:1; TODO: FIXME: may or may not use runpages
	pthread_create (&thread, NULL, joinFunc, (void *)stj);
}

void Join::WaitUntilDone () {
	 pthread_join (thread, NULL);
}

void Join::Use_n_Pages (int runlen) {
	pages = runlen;

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
	pages = runlen;

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
	char* s = new char[16];
	if(attr.myType == Int){
		std::ostringstream ss;
		ss << sumInt;
		string tempStr = ss.str() + "|";
		s = (char * )tempStr.c_str(); 
	}
	else if(attr.myType == Double){
		std::ostringstream ss;
  		ss << sumDouble;
		string tempStr = ss.str() + "|";
		s = (char * )tempStr.c_str();
	}
	else { 
		cerr<<"Invalid type return from computeME->apply encountered in Sum thread func "<<endl;
	}	
	Schema tempSch("schsum", 1, &attr);
	Record testing;
	testing.ComposeRecord(&tempSch,s);
	outPipe->Insert(&testing);
	outPipe->ShutDown();
	//delVar(attr.name); //FIXME: check for error here.

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
	pages = runlen;

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
	pages = runlen;

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
	pages = runlen;

}



