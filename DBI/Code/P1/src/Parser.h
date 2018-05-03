#ifndef PARSER_H
#define	PARSER_H


#include "QueryPlanner.h"

/* Parser parsess the query using yyparse 

Parser has Catalog reference to use statistics
and two maps of tablename to Alias reference and alias to table name reference 

ASSUMPTION :  We are not checking for any parser language error. All querys are valid 
*/

extern "C" {
	
	int yyparse(void);   // defined in y.tab.c
}
class Catalog;
class Parser{
    Catalog *cat;
    map<string,string> tableToAlias;
    map<string,string> aliasToTable;    
public:
    Parser(Catalog *a){
        cat=a;
    }
    bool parseAndCheck();
};

#endif