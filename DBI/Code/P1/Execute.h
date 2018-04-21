#ifndef EXECUTE_H
#define	EXECUTE_H

#include "QueryPlanner.h"


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
    // void InorderPrint(QPElement *root);
    // void performoperation(QPElement *node);    
    // void clearpipe(QPElement *r);
};

#endif