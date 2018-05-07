
#ifndef COMPILER_H
#define	COMPILER_H
#include "QueryPlanner.h"
#include "Catalog.h"
#include "Compiler.h"
#include "Parser.h"
#include "Optimizer.h"
#include "Execute.h"
/*
    Comipler holds all the steps invoved in the execution query together
    Steps involved in Query execution are :
        Build the relations using catalog.cc and statistics.cc
        Parse the query using parser.cc which inturn uses yyparser.
        Optimize the query execution tree using Optimize.cc
        Execute the query using Execute.cc
    This is the abstract view of what ever is happening in the project 4 & 5

    //Compile has object reference to all other classes 
    it has a boolean array to keep track of what operations are present in query
    And reference to root of the QueryTree to pass on to execute.

*/
//Forward Reference because of errors
class Optimizer;
class parser;
class Execute;
class Catalog;

class Compiler
{
    Parser *par;
    Optimizer *opt;
    Execute *exec;
    Catalog *cat;
    bool qOps[QOPSSIZE];        
    QPElement *root;
    
public:    
    static bool runOrPrint;
    static char *outFile;
    
    Compiler(Parser *p, Optimizer *o,Execute *e, Catalog *c){
        par = p;
        opt = o;
        exec = e;
        cat = c;         
    }  
    bool chkParse();
    void Optimize();
    void ExecuteQ();
    void Compile();
};

#endif