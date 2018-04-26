#include "Compiler.h"
char *Compiler::outFile=NULL;
extern bool modifyData; 

void Compiler::Compile()
{
    
    if(chkParse()){
        if(modifyData){
            exec->executeDataQuery();
            return;
        }
        Optimize();
        exec->printTree(root);
        exec->executeQuery(root);
    }
    else{
        cout<<"Query is Wrong!!!";
    }
}

void Compiler::Optimize(){    
    if(qOps[Join]) 
        opt->findOrder();
    opt->makePlan();
    exec->setroot(root);
}


bool Compiler::chkParse()
{
    TypesOfOps typeOp;
    //check if the code parses correctly
    if(par->parseAndCheck()){
        for(int x=0;x<=QOPSSIZE;x++)  qOps[x]=false;
                            
        if(tables->next!=NULL){
            typeOp = Join;
            qOps[typeOp]=true;
        }
        if(finalFunction!=NULL){
            if(groupingAtts!=NULL){
                typeOp = GroupBy;
                qOps[typeOp]=true;
            }
            else{
                typeOp = Sum;
                qOps[typeOp]=true;
            }
        }            
        if( qOps[GroupBy]==false && qOps[Sum]==false){
            typeOp = Project;
            qOps[typeOp]=true;
        }
        if(distinctAtts!=0 ||distinctFunc!=0){
            typeOp = Distinct;
            qOps[typeOp]=true;
        }            
        opt->SetQueryOps(qOps,QOPSSIZE,&root);
        return true;
    }
    return false;
}
