
#include <iostream>
#include "QueryPlanner.h"
#include "ParseTree.h"

using namespace std;

extern "C" {
	
	int yyparse(void);   // defined in y.tab.c
}

int main () {
	//TODO: Help write main file please :)
	

	// yyparse();

	//TODO: 1. Create a stats file first

	//2. initialize catalog
	Catalog * cat = Catalog::instantiate();

	//3. initialize parser
	 Parser *par = new Parser(cat);

	//4. initialize optimizer
	Optimizer *opt = new Optimizer(cat);

	//5. Execute???
	Execute *exec = new Execute();

	//6. initialize Compiler
	Compiler *comp = new Compiler(par,opt,exec,cat);

	comp->Compile();



}


