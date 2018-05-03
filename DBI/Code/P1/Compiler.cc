#include "Compiler.h"
char *Compiler::outFile=NULL;
extern bool modifyData; 

//main waypoint to compile project 5 files
void Compiler::Compile()
{
    //first go through the parser
    if(chkParse()){
        //check the parsed flags to see if its a DDL type
        if(modifyData){
            //send for data ops as required
            exec->executeDataQuery();
            return;
        }
        //otherwise optimize the query and execute it
        Optimize();
        exec->init();


       // if(!runOrPrint)
        //exec->printTree(root);
        //exec->levelOrderPrint(root);


         exec->executeQuery(root);
         exec->printNDel();
    }
    else{
        cout<<"\nERROR:\nUnable to parse query. Please make sure the query meets our defined language.\n\n"<<endl;
    }
}

void Compiler::Optimize(){  

    //if the query has join keyword, send it as required  
    if(qOps[Join]) 
        opt->findOrder();
    //then make a plan based on the query
    opt->makePlan();
    //pipe out the result
    exec->setrootNPipe(root,opt->PipeNum);
}


bool Compiler::chkParse()
{
    TypesOfOps typeOp;
    //check if the code parses correctly, goes to parse object to check
    if(par->parseAndCheck()){
        //second level check, no need to find and collect type details in case its a DLL query 
        if(modifyData) return true;
        //set all flags as false at start.
        for(int x=0;x<=QOPSSIZE;x++)  qOps[x]=false;

        //check if a particular type of op is present 
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
        //set them to the query optimizer   
        opt->SetQueryOps(qOps,QOPSSIZE,&root);
        return true;
    }
    return false;
}
