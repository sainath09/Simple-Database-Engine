#ifndef DEFS_H
#define DEFS_H
#include <stdio.h>
#include <stdlib.h>
#include <iostream>


#define MAX_ANDS 20
#define MAX_ORS 20

#define PAGE_SIZE 131072
using namespace std;

enum Target {Left, Right, Literal};
enum CompOperator {LessThan, GreaterThan, Equals};
enum Type {Int, Double, String};

const char *settings = "test.cat";
string gl_cat,gl_dbfile,gl_tpch;

// void setup () {
// 	ifstream fp(settings);
// 	if (fp.is_open) {
//         string line;
// 		getline(fp,line);
// 		catalog_path = &mem[0];
// 		dbfile_dir = &mem[80];
// 		tpch_dir = &mem[160];
// 		char line[80];
// 		fgets (line, 80, fp);
// 		sscanf (line, "%s\n", catalog_path);
// 		fgets (line, 80, fp);
// 		sscanf (line, "%s\n", dbfile_dir);
// 		fgets (line, 80, fp);
// 		sscanf (line, "%s\n", tpch_dir);
// 		fclose (fp);
// 		if (! (catalog_path && dbfile_dir && tpch_dir)) {
// 			cerr << " Test settings file 'test.cat' not in correct format.\n";
// 			free (mem);
// 			exit (1);
// 		}
// 	}
// 	else {
// 		cerr << " Test settings files 'test.cat' missing \n";
// 		exit (1);
// 	}
// 	cout << " \n** IMPORTANT: MAKE SURE THE INFORMATION BELOW IS CORRECT **\n";
// 	cout << " catalog location: \t" << catalog_path << endl;
// 	cout << " tpch files dir: \t" << tpch_dir << endl;
// 	cout << " heap files dir: \t" << dbfile_dir << endl;
// 	cout << " \n\n";

// 	// s = new relation (supplier, new Schema (catalog_path, supplier), dbfile_dir);
// 	// p = new relation (part, new Schema (catalog_path, part), dbfile_dir);
// 	// ps = new relation (partsupp, new Schema (catalog_path, partsupp), dbfile_dir);
// 	// n = new relation (nation, new Schema (catalog_path, nation), dbfile_dir);
// 	// li = new relation (lineitem, new Schema (catalog_path, lineitem), dbfile_dir);
// 	// r = new relation (region, new Schema (catalog_path, region), dbfile_dir);
// 	// o = new relation (orders, new Schema (catalog_path, orders), dbfile_dir);
// 	// c = new relation (customer, new Schema (catalog_path, customer), dbfile_dir);
// }


unsigned int Random_Generate();


#endif

