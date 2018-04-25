#ifndef EXECUTE_H
#define	EXECUTE_H

#include "QueryPlanner.h"
#include "RelOp.h"
// #include "Catalog.h"
 #include "Compiler.h"
// #include "Parser.h"
// #include "Optimizer.h"
//#include "Execute.h"

//Execute is a class that holds the root of tree structure to execute.
// We printed the tre using execute's printTree function
//Other functions are required for assignemnt five
class Execute
{
    QPElement *root;    
    int numPipes;   
    Pipe **pipes;
    DBFile **dbfiles;
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
    Execute(){
        struct TableList *tempTable = tables;
        int totTables=0;
        while(tempTable){
            tempTable=tempTable->next;
            totTables++;
        }

        pipes = new Pipe*[numPipes+1];          
        for(int i=0;i<=numPipes;i++) pipes[i] = new Pipe(100);

        dbfiles = new DBFile*[totTables];
        for(int i=0;i<totTables;i++) dbfiles[i]=new DBFile();

        selectfile= new class SelectFile*[totTables];
        for(int i=0;i<totTables;i++) selectfile[i]=new class SelectFile();
        
        selectpipe = new class SelectPipe*[totTables];
        for(int i=0;i<totTables;i++) selectpipe[i]=new class SelectPipe();
        selectPipes = 0;

        join = new class Join*[totTables];
        for(int i=0;i<totTables;i++) join[i]=new class Join();
        numjoin = 0;
        
        groupby = new class GroupBy();

        project = new class Project();
        
        sum = new class Sum();
        
        dupremove = new class DuplicateRemoval();
    }
    
    void setroot(QPElement *_root)
    {
        root=_root;
    }
    void printTree(QPElement *root);
    //TODO: assignment 5
    void executeQuery(QPElement *root);
};

#endif