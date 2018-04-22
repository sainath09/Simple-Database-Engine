#ifndef PARSER_H
#define	PARSER_H


#include "QueryPlanner.h"
extern "C" {
	int yyfuncparse(void);   // defined in y.tab.c
}
class Catalog;
class Parser
{
    Catalog *cat;
    //FIXME: DELETE THESE IF NOT NEEDED
    unordered_map<string,string> tableToAlias;
    unordered_map<string,string> aliasToTable;    
    // string alias;
    // string attr;
public:
    Parser(Catalog *a)
    {
        cat=a;
    }
    bool parseAndCheck();
    
    bool chkErrors(); //TODO: Delete this function if not needed
    // void Split(string str);
    // bool isAttValid(string attr);
};

#endif