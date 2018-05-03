
#include <iostream>

#include "Catalog.h"
#include "Compiler.h"
#include "Parser.h"
#include "Optimizer.h"
#include "Execute.h"

using namespace std;


string gl_cat = "" ,gl_dbfile = "" ,gl_tpch = "";
const char *settings = "test.cat";
void setup () {
	std::ifstream fp(settings);
	if (fp.is_open()) {
        string line;
		getline(fp,line);
		gl_cat = line;
        getline(fp,line);
		gl_dbfile = line;
        getline(fp,line);
        gl_tpch = line;
	}
	else {
		cerr << " Test settings files 'test.cat' missing \n";
		exit (1);
	}
	cout << " \n** IMPORTANT: MAKE SURE TO READ THE INFORMATION BELOW **\n";
	cout<<	"\n-FILES: MAKE SURE THESE PATHS ARE CORRECT --------------------------------\n";
	cout << " catalog location: \t" << gl_cat << endl;
	cout << " tpch files dir: \t" << gl_tpch << endl;
	cout << " heap files dir: \t" << gl_dbfile << endl;
	cout<<	"\n-SET FLAGS  --------------------------------------------\n";
	cout<< "";
	cout << " \n\n";
}

int main () {

	char *fileName = "Statistics.txt";
		Statistics s;
        char *relName[] = { "part", "partsupp", "supplier", "nation", "region"};
        s.AddRel(relName[0],200000);
		s.AddAtt(relName[0], "p_partkey",200000);
		s.AddAtt(relName[0], "p_size",50);

		s.AddRel(relName[1], 800000);
		s.AddAtt(relName[1], "ps_suppkey",10000);
		s.AddAtt(relName[1], "ps_partkey", 200000);
		
		s.AddRel(relName[2],10000);
		s.AddAtt(relName[2], "s_suppkey",10000);
		s.AddAtt(relName[2], "s_nationkey",25);
		
		s.AddRel(relName[3],25);
		s.AddAtt(relName[3], "n_nationkey",25);
		s.AddAtt(relName[3], "n_regionkey",5);

		s.AddRel(relName[4],5);
		s.AddAtt(relName[4], "r_regionkey",5);
		s.AddAtt(relName[4], "r_name",5);
		
		s.Write(fileName);
	//1. set up project variables
	setup();
	//2. initialize catalog
	while(true){
		clock_t start_time;
		double time = 0.0;
		start_time = clock();
		Catalog * cat = Catalog::instantiate();
		cout<< "New Query(Press Ctrl+D to execute):";
		//3. initialize parser
		Parser *par = new Parser(cat);

		//4. initialize optimizer
		Optimizer *opt = new Optimizer(cat);

		//5. Execute???
		Execute *exec = new Execute(cat);

		//6. initialize Compiler
		Compiler *comp = new Compiler(par,opt,exec,cat);

		comp->Compile();

		time = (clock() - start_time)/(double)CLOCKS_PER_SEC;
		cout<<"\nTime to execute query:"<<time<<endl<<endl;
	}




}


