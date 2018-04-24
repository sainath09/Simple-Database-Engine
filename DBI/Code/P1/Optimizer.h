
#ifndef OPTIMIZER_H
#define	OPTIMIZER_H

#include "QueryPlanner.h"
#include "Catalog.h"
#include "Compiler.h"
#include "Parser.h"
#include "Optimizer.h"
#include "Execute.h"

#include <cstring>
/*
    Optimizer does all the work of query planning
    there are various functions that are described in optimizer.h to perform required operations
    - Optimizer object maintains maps of different key,value pairs as described in name of the map
    - queryOps is a boolean array keeing track of different operations present in query 
    - totTables,Size,catalog object and a global attr and aliasTableName are also tracked in optimizer object
    


    */
class Optimizer
{
    bool *queryOps;
    int size;
    int totTables;
     string resFromJoin;
    
     map<string,string> tableToAlias;
     map<string,string> aliasToTable;
     map<string,string> uniqueIdToTable;
     map<string,string> tableToUniqueId;
    map<string,vector<struct AndList *> > idsToAndListGeneric;
     map<string,vector<struct AndList *> > idsToAndListSelect;
    map<struct AndList*,vector<string> > andListToIds;
    
    map<string,EstResultNode *> idToEstRes;
    Catalog *cat;
    string aliasTableName;
    string attr;
    QPElement **qproot;
public:
    Optimizer(Catalog *a)
    {
        cat=a;
    }
    void findUniqueTableForAndList();
    void findAliasAndAttr(string str);
    string retTableName();
    
    void returnVectorizedAndList(string expr,string newtabl,vector<struct AndList *> &vec);
    bool isBothSeqSame(string first,string second);
    string getSeq(string a);
    // void delFrmEstHash(int len);
    Schema* joinScema(Schema *s1,Schema *s2);
    struct AndList* returnAndList(string str);
    void findOrder();
    void makePlan();
    void processAttrs();
    void ProcessFunction(struct FuncOperator *func );
    void orderMakerForGroupBy(OrderMaker *o,Schema *s);

    void SetQueryOps(bool *a,int b,QPElement ** c)
    {
        queryOps = a;
        size = b;
        qproot=c;
    }    
};


#endif