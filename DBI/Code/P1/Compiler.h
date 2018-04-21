
#ifndef COMPILER_H
#define	COMPILER_H
#include "QueryPlanner.h"

class Compiler
{
    Parser *par;
    Optimizer *opt;
    Execute *exec;
    Catalog *cat;
    bool qOps[QOPSSIZE];        
    QPElement *root;
    
public:    
    static bool runQueryFlag;
    static char *outFile;
    
    Compiler(Parser *_par, Optimizer *_opt,Execute *_exec, Catalog *_cat)
    {
        par=_par;
        opt=_opt;
        exec=_exec;
        cat=_cat;         
    }  
    bool chkParse();
    void Optimize();
    void Execute();
    void Compile();
    void RunDDLquery();   
};

#endif