
#ifndef OPTIMIZER_H
#define	OPTIMIZER_H

#include "QueryPlanner.h"

class Optimizer
{
     //FIXME: DELETE THESE IF NOT NEEDED
    bool *queryOps;
    int size;
    int totTables;
    // string resultJoinExpr;
    
     unordered_map<string,string> tableToAlias;
     unordered_map<string,string> aliasToTable;
     unordered_map<string,string> uniqueIdToTable;
     unordered_map<string,string> tableToUniqueId;
    unordered_map<string,vector<struct AndList *> > idsToAndListGeneric;
     unordered_map<string,vector<struct AndList *> > idsToAndListSelect;
    unordered_map<struct AndList*,vector<string> > andListToIds;
    
    unordered_map<string,EstResultNode*> idToEstRes;
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
    // bool isAnagram(string first,string second);
    void returnVectorizedAndList(string expr,string newtabl,vector<struct AndList *> &vec);
    // string getAnagram(string newrelList);
    // void delFrmEstHash(int len);
    // Schema *ConstructJSchema(Schema *s1,Schema *s2);
    struct AndList* returnAndList(string str);
    void findOrder();
    void makePlan();
    // void ChangeLists();
    // void ChangeFunctionList(struct FuncOperator *func );
    // void CreateGroupByOm(OrderMaker *o,Schema *s);

    void SetQueryOps(bool *a,int b,QPElement ** c)
    {
        queryOps = a;
        size = b;
        qproot=c;
    }    
};


#endif