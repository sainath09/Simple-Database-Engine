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
	pages = runlen;

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
	Pipe* inPipeL=stj->inPipeL ;
	Pipe* inPipeR=stj->inPipeR ;
	Pipe * outPipe=stj->outPipe ;
	CNF *selOp=stj->selOp ;
	Record *literal=stj->literal ;
	Schema* left = stj->left;
	Schema* right = stj->right;
	
	OrderMaker omL,omR;
	Pipe *bqL = new Pipe(PIPE_BUFFER);
	Pipe *bqR = new Pipe(PIPE_BUFFER);
	Record tempR,tempL,resRec;
	ComparisonEngine ce;
	int FLAG_L = 1;
	int FLAG_R = 1;
	int commonAtts = selOp->GetSortOrders(omL,omR);

	if(commonAtts <  1){
		//Nested as there are no common attributes
		vector<Record *> rightRec;
		FLAG_L = inPipeL->Remove(&tempL);
		FLAG_R = inPipeR->Remove(&tempR);
		int attrL = tempL.getNumAtts();
		int attrR = tempR.getNumAtts();		
		int* resultAttr = new int[attrL + attrR];
		for(int i = 0;i<attrL;i++) resultAttr[i] = i;
		for(int j = 0;j<attrR;j++) resultAttr[attrL + j] = j;
		while(FLAG_R){
			Record* temp = new Record();
			//tempR.Print(right);
			temp->Consume(&tempR);
			FLAG_R = inPipeR->Remove(&tempR);
			rightRec.push_back(temp);
		}
		int count  = 0 ;
		while(FLAG_L){
			cout<<count++<<endl;
			for(int i = 0;i<rightRec.size();i++){
				if(ce.Compare(&tempL,&omL,rightRec[i],&omR) == 0){
					// cout<<"...left record..."<<endl;
					// tempL.Print(left);
					// cout<<"....right record...."<<endl;
					// rightRec[i]->Print(right);
					// cout<<"......"<<endl;
					resRec.MergeRecords(&tempL,rightRec[i],attrL,attrR,resultAttr, attrL+attrR,attrL);
					outPipe->Insert(&resRec);
				}
			}
			FLAG_L = inPipeL->Remove(&tempL);
		}
		delete[] resultAttr;
	}else{
		//try to find using points of commonality
		// structBigQ* sbql = new structBigQ();
		// sbql->iPipe = inPipeL;
		// sbql->oPipe = bqL;
		// sbql->order = &omL;
		// sbql->runlen = 1; //Manually defined runlength as 10 here		
		// structBigQ* sbqr = new structBigQ();
		// sbqr->iPipe = inPipeR;
		// sbqr->oPipe = bqR;
		// sbqr->order = &omR;
		// sbqr->runlen = 1; //Manually defined runlength as 10 here
		// pthread_t threadLeft,threadRight;		
		// pthread_create(&threadLeft,NULL,runBigqFunc,(void *) sbql);
		// pthread_create(&threadRight,NULL,runBigqFunc,(void *) sbqr);
		// pthread_join(threadLeft,NULL);
		// pthread_join(threadRight,NULL);
		BigQ srbql(*inPipeL,*bqL,omL,1);
		BigQ srbqr(*inPipeR,*bqR,omR,1);

		int count = 0;
		FLAG_L = bqL->Remove(&tempL);
		FLAG_R = bqR->Remove(&tempR);
		int attrL = tempL.getNumAtts();
		int attrR = tempR.getNumAtts();		
		int* resultAttr = new int[attrL + attrR];
		for(int i = 0;i<attrL;i++) resultAttr[i] = i;
		for(int j = 0;j<attrR;j++) resultAttr[attrL + j] = j;
		
		while(FLAG_L && FLAG_R){ // FLAGS should automatically end this loop in case one pipe shutsdown
			int temp = ce.Compare(&tempL,&omL,&tempR,&omR);	
			//int temp = 12;
			//cout<<"......first records......"<<endl;
			//tempL.Print(left);
			// cout<<" left records"<<endl;
			// while(FLAG_L){
			// 	tempL.Print(left);
			// 	FLAG_L = bqL->Remove(&tempL);
			// }
			// cout<<"right records"<<endl;
			// while(FLAG_R){
			// 	tempR.Print(right);
			// 	FLAG_R = bqR->Remove(&tempR);
			// }


			// cout<<"...second record.."<<endl;
			// tempR.Print(right);
			// cout<<"TEMP :"<<temp<<endl;
			if(temp == -1 ){ 
				//cout<<"left is less:"<<count++<<endl;
				FLAG_L = bqL->Remove(&tempL);
			}
			else if(temp  == 1){
				//cout<<"right is less"<<count++<<endl;
				FLAG_R = bqR->Remove(&tempR);
			}
			else if(temp == 0){
				//get equal records from right pipe
				vector<Record *> rightRecords;
				Record* t_r = new Record();
				t_r->Copy(&tempR); 
				while( FLAG_R && ce.Compare(t_r,&omR,&tempL,&omL) == 0){ //FIXME: did a b == c comparision may need to do a == b across tables
					// cout<<"...right Records.."<<endl;
					// t_r->Print(right);
					rightRecords.push_back(t_r);
					t_r = new Record(); //TODO: there may be address issues may have to use tempPrev everywhere
					FLAG_R = bqR->Remove(t_r); 
				}
				//get equal records from left pipe
				vector<Record*> leftRecords;
				Record* t_l = new Record();
				t_l->Copy(&tempL);
				while( FLAG_L && ce.Compare(t_l,&omL,rightRecords[0],&omR) == 0){ //FIXME: did a b == c comparision may need to do a == b across tables
					// cout<<"...left Records.."<<endl;
					// t_l->Print(left);
					leftRecords.push_back(t_l);
					t_l = new Record(); //TODO: there may be address issues may have to use tempPrev everywhere
					FLAG_L = bqL->Remove(t_l); 
				}

				for(int i = 0;i<leftRecords.size();i++){
					for(int j = 0;j<rightRecords.size();j++){
						// cout<<"......first records......"<<endl;
						// leftRecords[i]->Print(left);
						// cout<<"...second record.."<<endl;
						// rightRecords[j]->Print(right);
						// cout<<endl;
						
						resRec.MergeRecords(leftRecords[i],rightRecords[j],attrL,attrR,resultAttr,attrL + attrR,attrL);
						outPipe->Insert(&resRec);

					}
				}
				//FIRST APPOACH. DONT KNOW WHY IT IS NOT WORKING

				// while(FLAG_L && ce.Compare(&t_l,&omL,&tempR,&omR) == 0){
				// 	for(int i = 0;i<rightRecords.size();i++){
				// 		if (ce.Compare(&t_l, rightRecords[i], literal,selOp)) {
				// 			resRec.MergeRecords(&t_l,rightRecords[i],attrL,attrR,resultAttr,attrL + attrR,attrL);
				// 			outPipe->Insert(&resRec);
				// 		}
				// 	}
				// 	FLAG_L = bqL->Remove(&t_l);
				// }
				if(FLAG_R) tempR.Copy(t_r);
				if(FLAG_L) tempL.Copy(t_l);
				for(int i = 0;i<rightRecords.size();i++) delete rightRecords[i];
				for(int i = 0;i<leftRecords.size();i++) delete leftRecords[i];
				rightRecords.clear();
				
			}
		
		}
		delete[] resultAttr;
	}
	outPipe->ShutDown();
	bqL->ShutDown();
	bqR->ShutDown();
	delete bqL;
	delete bqR;


}

void Join::Run (Pipe &inPipeL, Pipe &inPipeR, Pipe &outPipe, CNF &selOp, Record &literal, Schema* left,Schema* right){
	stJoin *stj = new stJoin();
	stj->inPipeL = &inPipeL;
	stj->inPipeR = &inPipeR;
	stj->outPipe = &outPipe;
	stj->selOp = &selOp;
	stj->literal = &literal;
	stj->left = left;
	stj->right = right;
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
	Pipe* tempOpPipe = new Pipe(PIPE_BUFFER);
	
	BigQ sbq(*inPipe,*tempOpPipe,om,1);

	bool FLAG_FIRST = true;
	while(tempOpPipe->Remove(&nextRec)){
		if(FLAG_FIRST){
			tempRec.Copy(&nextRec);
			FLAG_FIRST = false;
		}
		else{
			if(ce.Compare(&nextRec,&tempRec,&om) != 0){
				//do not insert if both are equal
				outPipe->Insert(&tempRec);
				tempRec.Copy(&nextRec);
			}
		}

	}
	//insert last record
	outPipe->Insert(&tempRec);
	outPipe->ShutDown();
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
	char* s;
	ComparisonEngine ce;
	Schema tempsch("sum", 1, &attr);
	string intStr = "";
	string doubleStr = "";
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
					std::ostringstream ss;
					ss << sumInt;
					intStr += ss.str() + "|";
					s = (char *)intStr.c_str();
					sumInt = temp_int;
				}
				else if(attr.myType == Double){
					std::ostringstream ss;
					ss << sumDouble;
					doubleStr += ss.str() + "|";
					s = (char *)doubleStr.c_str();
					//sprintf(s,"%f|",sumDouble); //TODO: remove this line
					sumDouble = temp_double;
				}
				else { 
					cerr<<"Invalid type return from computeME->apply encountered in group by thread func "<<endl;
				}
				//FIXME: s should be cleared before this.
				Record sumRec;
				Record resRec;
				sumRec.ComposeRecord(&tempsch,s);
				tempRec2.Project (attArray, gbyNumAtts, currNumAtts);
				resRec.MergeRecords (&sumRec, &tempRec2, 1, gbyNumAtts, gbyAList, gbyNumAtts+1, 1);
				outPipe->Insert(&resRec);

			}
		}
		tempRec2.Consume(&tempRec);
	}

	if(attr.myType == Int) {
		std::ostringstream ss;
		ss << sumInt;
		intStr += ss.str() + "|";
		s = (char * )intStr.c_str(); 
	}
	else if(attr.myType == Double){
		std::ostringstream ss;
		ss << sumDouble;
		doubleStr += ss.str() + "|";
		s = (char * )doubleStr.c_str();
	} 
	else { 
		cerr<<"Invalid type return from computeME->apply encountered in group by thread func "<<endl;
	}
	//FIXME: s should be cleared before this.
	Record sumRec;
	Record resRec;
	sumRec.ComposeRecord(&tempsch,s);
	tempRec2.Project (attArray, gbyNumAtts, currNumAtts);
	resRec.MergeRecords (&sumRec, &tempRec2, 1, gbyNumAtts, gbyAList, gbyNumAtts+1, 1);
	outPipe->Insert(&resRec);
	outPipe->ShutDown();
	//delVar(attr.name);
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


