#include "Compiler.h"

void Compiler::Compile()
{
    
    if(chkParse()){
        //TODO:implement in ass5
        // if(DDL)
            // return;
        Optimize();
        exec->printTree(root);
    }
    else
    {
        cout<<"Parser Error:Syntax or Semantic issue";
   
    }
}

void Compiler::Optimize(){    
    if(qOps[Join]) opt->findOrder();
    opt->makePlan();
    exec->setroot(root);
}


bool Compiler::chkParse()
{
    TypesOfOps typeOp;
    //check if the code parses correctly
    if(par->parseAndCheck())
    {
        //TODO: For running actual queries
    //  if(DDL)
    //  {
    //      RunDDLquery();
    //      return true;
    //  }
     
    for(int x=0;x<=QOPSSIZE;x++)
         {
            qOps[x]=false;
        }
        if(par->chkErrors())
        {                    
            if(tables->next!=NULL)
            {
                typeOp = Join;
                qOps[typeOp]=true;
            }
            if(finalFunction!=NULL)
            {
                if(groupingAtts!=NULL)
                {
                    typeOp = GroupBy;
                    qOps[typeOp]=true;
                }
                else
                {
                    typeOp = Sum;
                    qOps[typeOp]=true;
                }
            }            
            if( qOps[GroupBy]==false && qOps[Sum]==false)
            {
                typeOp = Project;
                qOps[typeOp]=true;
            }
            if(distinctAtts!=0 ||distinctFunc!=0)
            {
                typeOp = Distinct;
                qOps[typeOp]=true;
            }            
            opt->SetQueryOps(qOps,QOPSSIZE,&root);

           return true;
        }
    }
    return false;
}
