#include "Parser.h"

bool Parser::parseAndCheck(){
    /* parseAndCheck Is a function atht parses the query using yyparse 
        it reveres the table list taht it created and populates the alias and tablename
        maps

    */
    int flag = yyparse();
	//tables is described in queryplanner
    struct TableList * tempA = tables;
    struct TableList * tempB ;
    struct TableList * lastState = NULL;
    //create the entire linked list of tables and alias names in reverse so filo is eliminated
    while(tempA != NULL) {
        tempB = tempA->next;
        tempA->next = lastState;
        lastState = tempA;
        tempA = tempB;
    }
    //flipping the linked list to remove filo
    tables = lastState;
    if(flag==0){
        //push these values to two hashmaps
        struct TableList *tableList = tables;
        while(tableList){
            tableToAlias[string(tableList->tableName)]= string(tableList->aliasAs);
            aliasToTable[string(tableList->aliasAs)]=string(tableList->tableName);
            tableList = tableList->next;
        }
    }
    return (flag == 0);
}
