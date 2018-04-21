#include "Optimizer.h"
using namespace std;

void Optimizer::findOrder()
{

    Statistics *statsPtr = cat->stats;  

    findUniqueTableForAndList();


    //put all single relations;
   
    for(auto it = andListToIds.begin();it!=andListToIds.end();it++){
        //if there are more than one tables ... dont bother  .. doing that afterwards
        if(it->second.size()==1)
            idsToAndListGeneric[it->second[0]].push_back(it->first);

    }

    //put all two relations;
    struct TableList *tHead=tables;
    struct TableList *tHeadNext = NULL;

    while(tHead!=NULL)
    {
        string first = tableToUniqueId[string(tHead->tableName)];
        tHeadNext = tHead->next;
        while(tHeadNext!=NULL){
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
    
    for(int i=0;i<totTables;i++,it++) idNames[i]=it->first;
    
    int i = 3;
    auto estItr=idToEstRes.begin();
    
    while(i <= totTables){
        estItr = idToEstRes.begin();
        while(estItr != idToEstRes.end() ){
            if(estItr->first.length() != (i-1) ){
                estItr++;
                continue;
            }
            string relList = estItr->first;
            for(int k =0;k<totTables;k++)
            {
                if(relList.find(idNames[k]) == string::npos)
                {
                    string newrelList = relList + idNames[k];
                    char **rnames = new char*[i];
                    for(int j=0;j<i;j++)
                    {
                        char x[2]={newrelList.at(j),'\0'};                       
                        string table = uniqueIdToTable[string(x)];                        
                        rnames[j]=new char[tableToAlias[table].length()+1];
                       // cout<<"\n tableAliasHashDS[table]"<<tableAliasHashDS[table];
                        strcpy(rnames[j],tableToAlias[table].c_str());
                        //cout<<"\n RelName:"<<rnames[j];
                        
                    }
                    vector<struct AndList *> vec;
                    returnVectorizedAndList(relList,idNames[k],vec);
                    struct AndList *head = returnAndList(newrelList);
                    double val = statsPtr->Estimate(head,rnames,i);
                    double cost = idToEstRes[estItr->first]->estimateTuples+idToEstRes[estItr->first]->costEstimate;
                    string angram=getAnagram(newrelList);
                    cout<<"\n expression:"<<newrelList<<" cost:"<<cost<<" estimate:"<<val;
                   if(angram.compare("")==0)
                    {
                    QueryExecStatsNode *node = new QueryExecStatsNode(newrelList,val,cost);
                    idToEstRes[newrelList]=node;
                    }
                    else if(idToEstRes[angram]->costEstimate>cost)
                    {
                        idToEstRes[angram]->costEstimate=cost;
                        idToEstRes[angram]->expr=newrelList;
                        idToEstRes[angram]->estimateTuples=val;
                    }
                }
            }
            estimateItr++;
        }
        //delete all length i-1;
        delFrmEstHash(i-1);
        i++;
    }
    estimateItr=queryEstimateHash.begin();
    
    cout<<"\n\n Printing Final List";
    while(estimateItr!=queryEstimateHash.end() )
    {
        cout<<"\n expression:"<<estimateItr->second->expr<<" cost:"<<estimateItr->second->costEstimate<<" estimate:"<<estimateItr->second->estimateTuples;
        resultJoinExpr = estimateItr->second->expr;
        estimateItr++;
    }

 //   cout<<"\n End Of Generating Join Order";
 
}


void Optimizer::returnVectorizedAndList(string a,string b,vector<struct AndList *> &v)
{
    int remaining;
    int length;    
    string tblList=a+b;

   for(auto it=andListToIds.begin();it!=andListToIds.end();it++)
    {
        // start processing on the andlist
        //find some basic info
        remaining=it->second.size();
        length=remaining;        
        if(remaining!=1)
        {
            //if there is only one, no need for this function to be called
            for(int i=0;i<length;i++)
            {
                //now start another iterator for the checking all values in the vector of unique Ids
                int tempSize=tblList.size();
                string temptabl=it->second[i];      
                //the below iterator goes thro all values in tblList          
                for(int j=0;j<tempSize;j++)
                {
                    //go thru horizontally in the long string on left and vertically in vector on right
                    if(tblList[j]==temptabl[0])
                        remaining--;
                }
            }
            //all values should match up at this point
            if(remaining==0)
            {
                //now... sigh ... work on these andlists till its looks correct
                bool flag=true;
                vector<struct AndList *>::iterator tempIt;
                while(a.length()!=0)
                {
                
                    if(idsToAndListGeneric.find(a)!=idsToAndListGeneric.end())
                    {
                    tempIt=idsToAndListGeneric[a].begin();

                        while(tempIt!=idsToAndListGeneric[a].end())
                        {
                
                            if(*tempIt==it->first)
                            {
                                flag=false;
                                break;
                            }
                            tempIt++;
                        }
                    }
                if(!flag)
                {
                    break;
                }
            
                string temp=a.substr(0,a.length()-1);;
                a=temp;
                    
                }
                if(flag==true)
                {
                    v.push_back(it->first);
                }
            }
        }
        
    }
    idsToAndListGeneric[tblList]=v;

}


struct AndList* Optimizer::returnAndList(string str)
{
    int size = str.length();

    //TODO:
    //change names as required... hash name already changed.
    //iterator changed too
    
    struct AndList dummy;
    dummy.rightAnd=NULL;
    struct AndList *temp=&dummy;
    //cout<<"\n str size:"<<size<<" "<<str;
    auto it = idsToAndListGeneric.begin();

    for(int i=0;i<size;i++)
    {
        //cout<<"\n substring"<<str.substr(i,i);
       // cout<<"\n substring"<<str.at(i);
        char x[2]={str.at(i),'\0'};


       it = idsToAndListGeneric.find(string(x));
       

       if(it != idsToAndListGeneric.end())
       {
           //cout<<"\nIn Create ANDLIST queryExec find on "<<x;
           int veclength=it->second.size();
     
           for (int j=0;j<veclength;j++)
           {
                temp->rightAnd=(struct AndList*)malloc(sizeof(struct AndList));
                temp=temp->rightAnd;
                temp->rightAnd=NULL;
                temp->left=it->second.at(j)->left;
        
           }           
       }       
    }
    for(int i=2;i<=size;i++)
    {
        
        if(idsToAndListGeneric.find(str.substr(0,i))!=idsToAndListGeneric.end())
        {
        int veclength=idsToAndListGeneric[str.substr(0,i)].size();
      
        for(int j=0;j<veclength;j++)
        {
           temp->rightAnd=(struct AndList*)malloc(sizeof(struct AndList));
           temp=temp->rightAnd;
           temp->rightAnd=NULL;           
                temp->left=idsToAndListGeneric[str.substr(0,i)].at(j)->left;
        }
        }            
    }


    return dummy.rightAnd;
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
        
        string s = "A"+i;
        uniqueIdToTable[s] = table;
        tableToUniqueId[table]=s;
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
            if(uniqueIds.size()>1)
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
            while(orlist!=NULL)
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
        while(tableList!=NULL){
            TblLnkList* ptr = attIt->second;
            string table = string(tableList->tableName);
            while(ptr!=NULL){
                if(table.compare(ptr->tableName)==0) return table;                    
                ptr = ptr->next;
            }
        tableList=tableList->next;
        }
    }   
}