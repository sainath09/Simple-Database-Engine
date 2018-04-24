#ifndef EXECUTE_H
#define	EXECUTE_H

#include "QueryPlanner.h"
#include "Catalog.h"
#include "Compiler.h"
#include "Parser.h"
#include "Optimizer.h"
//#include "Execute.h"

//Execute is a class that holds the root of tree structure to execute.
// We printed the tre using execute's printTree function
//Other functions are required for assignemnt five
class Execute
{
    QPElement *root;    
    int noofPipes;        

public:
    Execute()
    {
    }
    
    void setroot(QPElement *_root)
    {
        root=_root;
    }
    void printTree(QPElement *root);
    //TODO: assignment 5
};

#endif