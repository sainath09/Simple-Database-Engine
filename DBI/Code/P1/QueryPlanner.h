#ifndef QUERYPLANNER_H
#define	QUERYPLANNER_H

#define QOPSSIZE 10

#include <algorithm>
#include <iostream>
#include <stack>
#include <stdio.h>
#include <unordered_map>
#include <string>
#include <vector>
#include "Statistics.h"
#include "Defs.h"
#include "Schema.h"
#include "Function.h"
#include "ParseFunc.h"
#include "Comparison.h"
#include "Pipe.h"
#include "RelOp.h"
//#include "Catalog.h"
// #include "Compiler.h"
// #include "Parser.h"
// #include "Optimizer.h"
//#include "Execute.h"

static Attribute DobA = {"double",Double};
static Attribute StrA = {"string",String};
static Attribute IntA = {"int",Int}; 
static string enumvals[7]={"Project", "GroupBy", "Sum", "Join","Distinct","SelectFile","SelectPipe"};
static string types[3]={"Int", "Double", "String"};




using namespace std;

// extern "C" {
// 	int yyfuncparse(void);   
// }
extern	struct FuncOperator *finalFunction; 
extern	struct TableList *tables; 
extern	struct AndList *boolean; 
extern	struct NameList *groupingAtts; 
extern	struct NameList *attsToSelect; 
extern	int distinctAtts; 
extern	int distinctFunc;  


//Class to deal with project op
class ProjectAtts
{
public:
    int *attsToKeep;
    int numAttsIn;
    int numAttsOut;

    //initialize the values
 ProjectAtts(int *a, int b, int c)
 {
     attsToKeep = a;
     numAttsIn = b;
     numAttsOut = c;
 }
};

class EstResultNode{
public:
    string exp;
    double numTuples;
    double calcCost;
    //Constructor for estimation initialize all the values 
    EstResultNode(string a,double b,double c)
    {
        exp = a;
        numTuples = b;
        calcCost = c;
    }
};




//  types of operations, put in enum for simplicity

enum TypesOfOps {
     Project, 
     GroupBy, 
     Sum, 
     Join,
     Distinct,
     SelectFile,
     SelectPipe
};


//Just the tree element structure, not defining any functions specific to it here
class QPElement
{
    
public:
//need these ones as part of project description
    int inPipe1;
    int inPipe2;
    int outPipe;
    Schema *outSchema;
    CNF *cnf;

//made these up to help with code
    TypesOfOps typeOps;
    string tblName;
    string aliasName;
    Record *tempRec;
    Function *func;
    OrderMaker *om;
    QPElement *left;
    QPElement *right;
    ProjectAtts *pAtts;
//initializing values
    QPElement(TypesOfOps a,CNF *b,Schema *c,Function *d, OrderMaker *e, string f, string g, ProjectAtts *h, Record *j)
    {
        typeOps = a;
        cnf =b;
        outSchema = c;
        func=d;
        om=e;  
        tblName  = f;
        aliasName = g;
        pAtts = h;
        tempRec = j;
        //initializing pipes and pointers
        left=NULL;
        right=NULL;
        inPipe1=-1;
        inPipe2=-1;
        outPipe=-1;
    
    }
};

#endif	

