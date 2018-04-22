#ifndef EXECUTE_H
#define	EXECUTE_H

#include "QueryPlanner.h"
#include "Catalog.h"
#include "Compiler.h"
#include "Parser.h"
#include "Optimizer.h"
//#include "Execute.h"


class Execute
{
    QPElement *root;    
    int noofPipes;        

public:
    Execute()
    {
        //db = NULL;
    }
    //  void RunQuery();
    void setroot(QPElement *_root)
    {
        root=_root;
    }
    void printTree(QPElement *root);
    // void performoperation(QPElement *node);    
    // void clearpipe(QPElement *r);
};

#endif