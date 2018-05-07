#ifndef EXECUTE_H
#define	EXECUTE_H

#include "QueryPlanner.h"
#include "RelOp.h"
#include "Catalog.h"
 #include "Compiler.h"
 #include <fstream>
// #include "Parser.h"
// #include "Optimizer.h"
//#include "Execute.h"

//Execute is a class that holds the root of tree structure to execute.
// We printed the tree using execute's printTree function
//Other functions are required for assignemnt five, they are included in execute data query
class Execute
{
    QPElement *root;    
    int numPipes;   
    Pipe **pipes;
    DBFile **dbfiles;
    Catalog *cat;
    class SelectFile **selectfile;
    class SelectPipe **selectpipe;
    int selectPipes;
    class Join **join;
    int numjoin;
    class GroupBy *groupby;
    class Project *project;
    class Sum *sum;
    DuplicateRemoval *dupremove;
    int currentDBFile;
    int totTables;

public:
    Execute(Catalog *a){
        cat = a;
    }

    void init();
    
    void setrootNPipe(QPElement *_root,int numPipe);
    
    void printTree(QPElement *root);
    void executeQuery(QPElement *root);
    
    void executeDataQuery();

    void printNDel();
    void levelOrderPrint(QPElement* root);
};

#endif