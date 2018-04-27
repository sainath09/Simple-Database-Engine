#include "Optimizer.h"
using namespace std;

void Optimizer::findOrder()
{

    Statistics *statsPtr = cat->stats;  

    findUniqueTableForAndList();
   
    for(auto it = andListToIds.begin();it!=andListToIds.end();it++){
        //if there are more than one tables ... dont bother  .. doing that afterwards
        if(it->second.size()==1)
            idsToAndListGeneric[it->second[0]].push_back(it->first);

    }

    //put all two relations;
    struct TableList *tHead=tables;
    struct TableList *tHeadNext = NULL;

    while(tHead)
    {
        string first = tableToUniqueId[string(tHead->tableName)];
        tHeadNext = tHead->next;
        while(tHeadNext){
            vector<struct AndList *> vec;
            string second = tableToUniqueId[string(tHeadNext->tableName)];
            //spread out andlist to all the tables and return in it back in vec vector of Andlist
            returnVectorizedAndList(first,second,vec);

            //make variables for stats compute
            char **rnames = new char*[2];
            rnames[0] = tHead->aliasAs;
            rnames[1] = tHeadNext->aliasAs;

            struct AndList *head = returnAndList(first+second);
            double val = statsPtr->Estimate(head,rnames,2);                
            EstResultNode *node = new EstResultNode(first+second,val,0);
            idToEstRes[first+second]=node;
            tHeadNext=tHeadNext->next;
        }
        tHead=tHead->next;
    }
    string idNames[totTables];
    auto it=uniqueIdToTable.begin();
    
    for(int i=0;i<totTables;i++,it++) {
        //cout<<it->first<<it->second<<endl;
        idNames[i]=it->first;
        //cout<<idNames[i]<<endl;
    }
    auto estItr=idToEstRes.begin();
    
    for(int i = 3;i <= totTables;i++){
        estItr = idToEstRes.begin();
        for(;estItr != idToEstRes.end();estItr++ ){
            if((int)estItr->first.length() != (i-1) ){
                estItr++;
                continue;
            }
            string relationName = estItr->first;
            for(int k =0;k<totTables;k++){
                if(relationName.find(idNames[k]) == string::npos){
                    string newrelationName = relationName + idNames[k];
                    char **rnames = new char*[i];
                    for(int j=0;j<i;j++){   
                        char tempChar[2]={newrelationName[j],'\0'}; 

                        string table = uniqueIdToTable[string(tempChar)];                        
                        rnames[j]=new char[tableToAlias[table].length()+1];
                        strcpy(rnames[j],tableToAlias[table].c_str());
                    }
                    vector<struct AndList *> vec;
                    returnVectorizedAndList(relationName,idNames[k],vec);
                    struct AndList *head = returnAndList(newrelationName);
                    double val = statsPtr->Estimate(head,rnames,i);
                    double cost = idToEstRes[estItr->first]->numTuples+idToEstRes[estItr->first]->calcCost;
                    string sequence = "";
                    sequence = getSeq(newrelationName);
                    cout<<"\n expression:"<<newrelationName<<" cost:"<<cost<<" estimate:"<<val;
                    if(sequence.compare("")==0){
                        EstResultNode *node = new EstResultNode(newrelationName,val,cost);
                        idToEstRes[newrelationName]=node;
                    }
                    else if(idToEstRes[sequence]->calcCost>cost){
                        idToEstRes[sequence]->calcCost=cost;
                        idToEstRes[sequence]->exp=newrelationName;
                        idToEstRes[sequence]->numTuples=val;
                    }
                }
            }
        }
        //delete all length i-1;
        //Deleting all ids with length less than i - 1
        int len = i - 1;
        estItr=idToEstRes.begin();
        for(;estItr!=idToEstRes.end();estItr++){
            if((int)estItr->first.length()==len){
                delete estItr->second;
                string temp=estItr->first;
                idToEstRes.erase(temp);
            } 
        }
    }
    estItr=idToEstRes.begin();
    //keep track of name 
    cout<<"......Final Query....."<<endl;
    for(;estItr!=idToEstRes.end();estItr++ ){
        if(estItr->first.length() < totTables) continue; //TODO: work around
        cout<<"\n expression:"<<estItr->second->exp<<" cost:"<<estItr->second->calcCost<<" estimate:"<<estItr->second->numTuples;
        resFromJoin = estItr->second->exp;
        // cout<<resFromJoin;
    }
}

void Optimizer::returnVectorizedAndList(string a,string b,vector<struct AndList *> &v)
{
    int remaining;
    int length;    
    string tblList=a+b;
    int tempSize=tblList.size();
    string tempFinal = tblList;
    for(auto it=andListToIds.begin();it!=andListToIds.end();it++){
        // start processing on the andlist
        //find some basic info
        remaining=it->second.size();
        length=remaining;        
        if(remaining!=1){
            //if there is only one, no need for this function to be called
            for(int i=0;i<length;i++){
                //now start another iterator for the checking all values in the vector of unique Ids
                
                string temptabl=it->second[i];      
                //the below iterator goes thro all values in tblList          
                for(int j=0;j<tempSize;j++){
                    //go thru horizontally in the long string on left and vertically in vector on right
                    if(tblList[j]==temptabl[0])
                        remaining--;
                }
            }
            //all values should match up at this point
            if(remaining==0){
                //now... sigh ... work on these andlists till its looks correct
                bool flag=true;
                vector<struct AndList *>::iterator tempIt;
                while(tempFinal.length()!=0){
                    if(idsToAndListGeneric.find(tempFinal)!=idsToAndListGeneric.end()){
                        tempIt=idsToAndListGeneric[tempFinal].begin();
                        while(tempIt!=idsToAndListGeneric[tempFinal].end()){
                            if(*tempIt==it->first){
                                flag=false;
                                break;
                            }
                            tempIt++;
                        }
                    }
                if(!flag) break;
                string temp=tempFinal.substr(0,tempFinal.length()-1);
                tempFinal=temp; 
                }
                if(flag) v.push_back(it->first);
            }
        }
        
    }
    idsToAndListGeneric[tblList]=v;

}


struct AndList* Optimizer::returnAndList(string str)
{
    //function creates and returns the Andlist.
    int size = str.length();
    struct AndList retVal; // and list to return 
    retVal.rightAnd=NULL; //initializing rightAnd of andlist
    struct AndList *temp=&retVal; //temp variable to parse the tree.
    auto it = idsToAndListGeneric.begin();
    for(int i=0;i<size;i++){
        char tempChar[2]={str.at(i),'\0'};
       it = idsToAndListGeneric.find(string(tempChar));
       if(it != idsToAndListGeneric.end()){
           for (int j=0;j<(int)it->second.size();j++) {
               //looking over all the string values in the hash
                temp->rightAnd=(struct AndList*)malloc(sizeof(struct AndList));
                temp=temp->rightAnd;
                temp->rightAnd=NULL;
                temp->left=it->second[j]->left;
           }           
       }       
    }
    for(int i=2; i <= size; i++){
        if(idsToAndListGeneric.find(str.substr(0,i))!=idsToAndListGeneric.end()){
            int veclength=idsToAndListGeneric[str.substr(0,i)].size();
            for(int j=0;j<veclength;j++){
                temp->rightAnd=(struct AndList*)malloc(sizeof(struct AndList));
                temp=temp->rightAnd;
                temp->rightAnd=NULL;           
                temp->left=idsToAndListGeneric[str.substr(0,i)].at(j)->left;
            }
        }            
    }
    return retVal.rightAnd;
}

void Optimizer::findUniqueTableForAndList()
{
    //copy the global tables struc
     struct TableList *TempTable = tables;
     //copy stats too
     Statistics *statsPtr = cat->stats;
     int i=0;
    //go thru all the andlist in the table struc
     while(TempTable){

         //copy to hashlist and create unique id... similar to parse function... 
         //didnt want to create too many globals ... lol

        string table = string(TempTable->tableName);

        //these are the hashes
        tableToAlias[table]=string(TempTable->aliasAs);
        aliasToTable[string(TempTable->aliasAs)]=table;

        //copy the tablename relation to that of the alias too
        statsPtr->CopyRel(TempTable->tableName,TempTable->aliasAs);
         char c = 'A'+i;
        char cstr[2] = {c,'\0'};
        string s(cstr);
        
        uniqueIdToTable[s] = table;
        tableToUniqueId[table]=s;
        //s = "";
        cout<<"\n Table:"<<s<<" alias:"<<table;
        // increment iterator over temptable
         TempTable = TempTable->next;
        i++;
    }
    //count the number of overall tables operated upon the last action
    totTables = i;
    struct AndList *tempAndList = boolean;
    //again... copy the entire global andlist
    while(tempAndList)
    {        
        //separate the first left or
        struct OrList *orlist=tempAndList->left;
        bool FLAGOR = false;
        //if there are any right child ors 
        if(orlist->rightOr)
        {
            //lets start combining all the table ids... so we know which tables we are using for that particular andlist
            vector<string> uniqueIds;
            //iterate over all child ors to make the list
            while(orlist)
            {
            
                ComparisonOp *Op=orlist->left;
                if(Op->left->code==3)
                {
                    findAliasAndAttr(Op->left->value);
                    uniqueIds.push_back(tableToUniqueId[retTableName()]);
                }

                if(Op->right->code==3)
                {
                    findAliasAndAttr(Op->right->value);
                    uniqueIds.push_back(tableToUniqueId[retTableName()]);
                }
                orlist=orlist->rightOr;
            }
         // now make a list of all the tables we need
            sort(uniqueIds.begin(),uniqueIds.end());
            uniqueIds.erase(unique(uniqueIds.begin(),uniqueIds.end()),uniqueIds.end());
            //if there is more than one tables 
            if((int)uniqueIds.size()>1)
            {
                FLAGOR = true;
                string temp = "";
                //make a string of all the table names and push the Andlist to this long string
                for(i=0;i<uniqueIds.size();i++)
                {
                    temp +=  uniqueIds[i]; 
                }                
                idsToAndListSelect[temp].push_back(tempAndList);
                queryOps[SelectPipe] = true;
            }
        }
        //if there is only one table push uniquetable ID to andlist directly
        if(!FLAGOR){
            orlist=tempAndList->left;
            while(orlist)
            {
                ComparisonOp *Op=orlist->left;
                if(Op->left->code==3)
                {
                    findAliasAndAttr(Op->left->value);                
                    andListToIds[tempAndList].push_back(tableToUniqueId[retTableName()]);
                }
                
                if(Op->right->code==3)
                {
                    findAliasAndAttr(Op->right->value);
                    andListToIds[tempAndList].push_back(tableToUniqueId[retTableName()]);
                }

            orlist=orlist->rightOr;
            }
            //now sort and just keep uniques here too
            sort(andListToIds[tempAndList].begin(),andListToIds[tempAndList].end());
            andListToIds[tempAndList].erase(unique(andListToIds[tempAndList].begin(),andListToIds[tempAndList].end()),andListToIds[tempAndList].end());        
        }

        //call the next andlist
        tempAndList=tempAndList->rightAnd;
    }
}

void Optimizer::findAliasAndAttr(string str){
    size_t pos = str.find('.',0);
    if(pos==string::npos)
    {
        aliasTableName="";
        attr=str;
    }
    else
    {
    aliasTableName= str.substr(0,pos);
    attr = str.substr(pos+1);
    }

}

string Optimizer::retTableName()
{
    if(aliasTableName!="") return aliasToTable[aliasTableName];
    else{ 
        auto attIt = cat->attrToTable.find(attr);
        struct TableList *tableList=tables;
        while(tableList){
            TblLnkList* ptr = attIt->second;
            string table = string(tableList->tableName);
            while(ptr){
                if(table.compare(ptr->tableName)==0) return table;                    
                ptr = ptr->next;
            }
            tableList=tableList->next;
        }
    }   
}


string Optimizer::getSeq(string a)
{
    auto estItr=idToEstRes.begin();
    for(;estItr!=idToEstRes.end();estItr++){
        if(isBothSeqSame(a,estItr->first)) return estItr->first;
    }
    return "";
}

bool Optimizer::isBothSeqSame(string first,string second)
{
    bool flag;
    if((int)first.size() == (int)second.size()){
        sort(first.begin(),first.end());
        sort(second.begin(),second.end());
        cout<<first <<" "<<second<<endl;
        for(int i = 0;i<(int)first.size();i++){
            if(first[i] != second[i]) return false;
        }
    }
    else return false;
    cout<<first<< " "<<second<<" Returning true"<<endl;
    return true;
}

// Actual query plan starts here

void Optimizer::orderMakerForGroupBy(OrderMaker *o,Schema *s){    
    int i = 0;    
    struct NameList *temp = groupingAtts;
    while(temp!=NULL){
        int res = s->Find(temp->name);
        Type restype = s->FindType(temp->name);
        if(res!=-1){
            o->whichAtts[i] = res;
            o->whichTypes[i] = restype;
            i++;
        }
        temp = temp->next;
    }
    o->numAtts=i;                
}

void Optimizer::processAttrs()
{
    struct AndList* head=boolean;
    while(head){
        struct OrList* orlist = head->left;
        while(orlist){
            if(orlist->left->left->code == 3){
                findAliasAndAttr(orlist->left->left->value);
                strcpy(orlist->left->left->value,attr.c_str());
            }
            if(orlist->left->right->code == 3){
                findAliasAndAttr(string(orlist->left->right->value));
                strcpy(orlist->left->right->value,attr.c_str());
            }
            orlist=orlist->rightOr;
        }
        head=head->rightAnd;
    }
    struct NameList *atts=groupingAtts;
    while(atts){
        findAliasAndAttr(string(atts->name));
        strcpy(atts->name,attr.c_str());
        atts=atts->next;
    }
    atts=attsToSelect;
    while(atts){
        findAliasAndAttr(string(atts->name));
        strcpy(atts->name,attr.c_str());
        atts=atts->next;
    }
    ProcessFunction(finalFunction);
}

void Optimizer::ProcessFunction(struct FuncOperator *func ){
    //do the same for function atts for all the nodes in the function structure
    if(!func)return;
    if(func->leftOperand){
        findAliasAndAttr(string(func->leftOperand->value));
        strcpy(func->leftOperand->value,attr.c_str());
    }
    ProcessFunction(func->leftOperator);
    ProcessFunction(func->right);
    
}

Schema* Optimizer::joinScema(Schema *s1,Schema *s2)
{
    int numAtts1 = s1->GetNumAtts();
    int numAtts2 = s2->GetNumAtts();
    Attribute *att1 = s1->GetAtts();
    Attribute *att2 = s2->GetAtts();
    int resultnumAtt = numAtts1 + numAtts2;
    Attribute *res = new Attribute[resultnumAtt];
    int i=0;
    for(i=0;i<numAtts1;i++)
    {
        res[i].myType = att1[i].myType;
        res[i].name = strdup(att1[i].name);
    }
    int j=i;
    for(i=0;i<numAtts2;i++,j++)
    {
        res[j].myType = att2[i].myType;
        res[j].name = strdup(att2[i].name);
    }
    Schema *s3 = new Schema("joined",resultnumAtt,res);
    return s3;
}
extern string gl_cat;
void Optimizer::makePlan(){

    processAttrs();
    QPElement *lchild=NULL;
    QPElement *rchild=NULL;    
    
    PipeNum=0;

    //----------FOR JOIN ...........

    if(queryOps[Join] == true){        
        string str1;
        str1.push_back(resFromJoin[0]);
        string tableName1 = uniqueIdToTable[str1];
        //char *tableName1 = new char[uniqueIdToTable[string(str1)].length()];
        //strcpy(tableName1,uniqueIdToTable[string(str1)].c_str());
        CNF *cnf1=new CNF();
        Schema *s1=new Schema((char *)gl_cat.c_str(),(char *) tableName1.c_str());
        Record *literal1 = new Record();
        int numRel=0;
        auto listItr = idsToAndListGeneric.begin();
        listItr = idsToAndListGeneric.find(resFromJoin.substr(numRel,1));
        if(listItr != idsToAndListGeneric.end()){                        
            struct AndList dummy;
            dummy.rightAnd=NULL;
            struct AndList *temp=&dummy;
            int size=listItr->second.size();
            for(int i=0;i<size;i++){
                temp->rightAnd = (struct AndList*)malloc(sizeof(struct AndList));
                temp = temp->rightAnd;
                temp->rightAnd = NULL;
                temp->left = listItr->second[i]->left;
            }
            cnf1->GrowFromParseTree(dummy.rightAnd,s1,*literal1);           
        }
        else{           
            cnf1 = NULL;
            literal1 = NULL;
        }
        
        QPElement *elem = new QPElement(SelectFile,cnf1,s1,NULL,NULL,tableName1,tableToAlias[tableName1],NULL,literal1);
        elem->left = NULL;
        elem->right = NULL;
        elem->outPipe=PipeNum;
        PipeNum++;
        lchild = elem;
        
        numRel++; 

        //Select Node 2:
        string str2;
        str2.push_back(resFromJoin[1]);
        string tableName2 = uniqueIdToTable[str2];
        
        
        CNF *cnf2=new CNF();
        Schema *s2=new Schema((char *)gl_cat.c_str(),(char *)tableName2.c_str());
        Record *literal2 = new Record();      
        listItr = idsToAndListGeneric.find(resFromJoin.substr(numRel,1));
        if(listItr!=idsToAndListGeneric.end()){
            struct AndList dummy;
            dummy.rightAnd=NULL;
            struct AndList *temp=&dummy;
            int vecs=listItr->second.size();
            for(int i=0;i<vecs;i++)
            {
            temp->rightAnd=(struct AndList*)malloc(sizeof(struct AndList));
            temp=temp->rightAnd;
            temp->rightAnd=NULL;
            temp->left=listItr->second.at(i)->left;
            }
            cnf2->GrowFromParseTree(dummy.rightAnd,s2,*literal2);           
        }
        else
        {
            cnf2=NULL;
            literal2=NULL;            
        }
 
        elem = new QPElement(SelectFile,cnf2,s2,NULL,NULL,tableName2,tableToAlias[string(tableName2)],NULL,literal2);
        elem->left=NULL;
        elem->right=NULL;
        elem->outPipe=PipeNum;
        PipeNum++;
        rchild = elem;

        numRel++;
        /* Code for Joining relations


        */

        //Loop and Do Join on the result with new Tables
        int numjoin=1;
        do{        
            Record *joinedLiteral = new Record();
            CNF *joinedCNF = new CNF();
            Schema *joinedScema = joinScema(lchild->outSchema,rchild->outSchema);        
            listItr = idsToAndListGeneric.find(resFromJoin.substr(0,numjoin+1));
            if(listItr!=idsToAndListGeneric.end()){
                struct AndList dummy;
                dummy.rightAnd=NULL;
                struct AndList *temp=&dummy;
                int vecs=listItr->second.size();
                for(int i=0;i<vecs;i++){
                    temp->rightAnd=(struct AndList*)malloc(sizeof(struct AndList));
                    temp=temp->rightAnd;
                    temp->rightAnd=NULL;
                    temp->left=listItr->second.at(i)->left;
                }
              joinedCNF->GrowFromParseTree(dummy.rightAnd,lchild->outSchema,rchild->outSchema,*joinedLiteral);              
            }   
            else{
                joinedCNF = NULL;
                joinedLiteral =NULL;
            }
            
            elem = new QPElement(Join,joinedCNF,joinedScema,NULL,NULL,"","",NULL,joinedLiteral);
            elem->left=lchild;
            elem->right=rchild;
            elem->inPipe1 = lchild->outPipe;
            elem->inPipe2 = rchild->outPipe;
            elem->outPipe = PipeNum;
            PipeNum++;
            lchild = elem;
            rchild = NULL;
            
            // Check For Select PIPE OP; If any problem *DELETE HERE*
            if(queryOps[SelectPipe]==true){
                
                struct AndList dummy;
                dummy.rightAnd=NULL;
                struct AndList *temp=&dummy;
                auto qSelItr=  idsToAndListSelect.begin();
                while(qSelItr!=idsToAndListSelect.end()){
                    string joinName=resFromJoin.substr(0,numjoin+1);
                    map<char,int> mp;
                    int len = qSelItr->first.length();
                    
                    for(int i=0;i<len;i++) mp[qSelItr->first[i]]=1;

                    len = joinName.size();
                    
                    for(int i=0;i<len;i++) mp.erase(joinName[i]);
                    
                    if(mp.empty()){

                        int size=qSelItr->second.size();
                        for(int i=0;i<size;i++)
                        {
                        temp->rightAnd=(struct AndList*)malloc(sizeof(struct AndList));
                        temp=temp->rightAnd;
                        temp->rightAnd=NULL;
                        temp->left=qSelItr->second.at(i)->left;
                        }
                        idsToAndListSelect.erase(qSelItr);
                    }
                    qSelItr++;
                }
                if(dummy.rightAnd!=NULL){
                    CNF *cs = new CNF();
                    Record *selectLiteral = new Record();
                    cs->GrowFromParseTree(dummy.rightAnd,lchild->outSchema,*selectLiteral);
                    elem = new QPElement(SelectPipe,cs,lchild->outSchema,NULL,NULL,"","",NULL,selectLiteral);
                    elem->left=lchild;
                    elem->right=NULL;
                    elem->inPipe1 = lchild->outPipe;            
                    elem->outPipe = PipeNum;
                    PipeNum++;
                    lchild = elem;
                }
            }

            numjoin++;
            if(numjoin<totTables){
                string str4;
                str4.push_back(resFromJoin[numjoin]);
                string tableName4 = uniqueIdToTable[str4];
                CNF *cnf4=new CNF();
                Schema *s4=new Schema((char *)gl_cat.c_str(),(char*) tableName4.c_str());
                Record *literal4 = new Record();              
                listItr = idsToAndListGeneric.find(resFromJoin.substr(numRel,1));              
                if(listItr!=idsToAndListGeneric.end()){
                    struct AndList dummy;
                    dummy.rightAnd=NULL;
                    struct AndList *temp=&dummy;
                    int size=listItr->second.size();
                    for(int i = 0;i < size;i++){
                        temp->rightAnd=(struct AndList*)malloc(sizeof(struct AndList));
                        temp=temp->rightAnd;
                        temp->rightAnd=NULL;
                        temp->left=listItr->second.at(i)->left;
                    }
                    cnf4->GrowFromParseTree(dummy.rightAnd,s4,*literal4);                   
                }
                else
                {
                    cnf4=NULL;
                    literal4=NULL;                    
                }
                elem = new QPElement(SelectFile,cnf4,s4,NULL,NULL,tableName4,tableToAlias[tableName4],NULL,literal4);
                elem->left=NULL;
                elem->right=NULL;
                elem->outPipe=PipeNum;
                PipeNum++;
                rchild = elem;
                numRel++;
            }
        }while(numjoin<totTables);        
    }
    else{
        CNF *cnf = new CNF();
        Schema *s=new Schema((char *)gl_cat.c_str(),tables->tableName);
        Record *literal = new Record();
        cnf->GrowFromParseTree(boolean,s,*literal);
        QPElement *newNode = new QPElement(SelectFile,cnf,s,NULL,NULL,string(tables->tableName),string(tables->aliasAs),NULL,literal);
        newNode->left=NULL;
        newNode->right=NULL;
        newNode->outPipe=PipeNum;
        PipeNum++;
        lchild = newNode;
        rchild = NULL;        
    }

    //Remaining Operations After Join:
     if(queryOps[GroupBy]==true)
     {
            Function *f = new Function();
            f->GrowFromParseTree(finalFunction,*lchild->outSchema);
            OrderMaker *om = new OrderMaker();
            orderMakerForGroupBy(om,lchild->outSchema);
            //Some output Schema; Anway we are not going to use the Output Schema from group
            //By Further;
            Schema *group_schema = new Schema("group_schema",1,&DobA);                        
            QPElement *elem = new QPElement(GroupBy,NULL,group_schema,f,om,"","",NULL,NULL);
            elem->left = lchild;
            elem->right = NULL;
            elem->inPipe1=lchild->outPipe;
            elem->outPipe=PipeNum;
            PipeNum++;
            lchild = elem;
            rchild = NULL;            
     }
     else if (queryOps[Sum]==true)
     {
            Function *f = new Function();            
            f->GrowFromParseTree(finalFunction,*lchild->outSchema);
            Schema *sum_schema = new Schema("sum_schema",1,&DobA);
            QPElement *newSumNode = new QPElement(Sum,NULL,sum_schema,f,NULL,"","",NULL,NULL);
            newSumNode->left = lchild;
            newSumNode->right = NULL;
            newSumNode->inPipe1=lchild->outPipe;
            newSumNode->outPipe=PipeNum;
            PipeNum++;
            lchild = newSumNode;
            rchild = NULL;
     }
     else if (queryOps[Project]==true)
        {
         int numAtts = lchild->outSchema->GetNumAtts();
         Attribute *atts = lchild->outSchema->GetAtts();
         struct NameList *attSel=attsToSelect;         
         int cnt=0;
         while(attSel!=NULL)
         {
             cnt++;
             attSel = attSel->next;
         }
         Attribute *projectoutAtts = new Attribute[cnt];
         attSel=attsToSelect;
         int *attsToKeep = new int[cnt];
         vector<int> vec;
         while(attSel!=NULL)
         {
          for (int i = 0,j=0; i < numAtts; i++) {
            if(strcmp(atts[i].name,attSel->name)==0)
             {            
              vec.push_back(i);
             }
            }
            attSel = attSel->next;
         }
         sort(vec.begin(),vec.end());
         for(int i=0;i<vec.size();i++)
         {
             attsToKeep[i]=vec.at(i);
             //cout<<"\n attsToKeep[i]:"<<attsToKeep[i];
             projectoutAtts[i].myType = atts[vec.at(i)].myType;
             projectoutAtts[i].name = strdup(atts[vec.at(i)].name);

         }
         Schema *ps = new Schema("ps",cnt,projectoutAtts);
         ProjectAtts *projectatts = new ProjectAtts(attsToKeep,numAtts,cnt);

         QPElement *newProjNode = new QPElement(Project,NULL,ps,NULL,NULL,"","",projectatts,NULL);
         newProjNode->left = lchild;
         newProjNode->right = NULL;
         newProjNode->inPipe1=lchild->outPipe;
         newProjNode->outPipe=PipeNum;
         PipeNum++;
         lchild = newProjNode;
         rchild=NULL;
        }

    if(queryOps[Distinct]==true && queryOps[Sum]==false)
    {
        //Use the output Schema as the Schema to use for Duplicate Removal
         QPElement *newDistNode = new QPElement(Distinct,NULL,lchild->outSchema,NULL,NULL,"","",NULL,NULL);
         newDistNode->left = lchild;
         newDistNode->right = NULL;
         newDistNode->inPipe1=lchild->outPipe;
         newDistNode->outPipe=PipeNum;
         PipeNum++;
         lchild = newDistNode;
         rchild=NULL;
    }
    *qproot = lchild;
}    