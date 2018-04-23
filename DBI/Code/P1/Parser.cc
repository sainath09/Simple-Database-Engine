#include "Parser.h"

bool Parser::parseAndCheck()
{
    //TODO: check if yyfuncparse is needed ... 
    // replace as necessary 
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


    if(flag==0)
    {
        //push these values to two hashmaps
        struct TableList *tableList = tables;
        //TODO: try to fix it if possible
        while(tableList!=NULL)
        {
            tableToAlias[string(tableList->tableName)]= string(tableList->aliasAs);
            aliasToTable[string(tableList->aliasAs)]=string(tableList->tableName);
            tableList = tableList->next;
        }
    }
    return (flag == 0);
}

//TODO: change func name .. DONE 
// ... ARE WE KEEPING THIS ONE? 
// It checks if the query is correct... we dont care right?
// no one asked us to check all that...
// maybe we do less indepth version
bool Parser::chkErrors()
{
// //Check if SelectAtts are all in groupAtts
//     struct NameList *groupingAttsitr=groupingAtts; // grouping atts (NULL if no grouping)
//     struct NameList *attsToSelectitr=attsToSelect;
//     struct TableList *tablelist=tables;
//     struct AndList *andList=boolean;
//     bool result=false;

//     map<string,string>::iterator tabItr;
//     map<string,CatalogTables*>::iterator attitr;
//     map<string,int> queryattribs;
    
//     while(tablelist!=NULL)
//     {
//        tabItr=myCatalog->catalogTableHash.find(string(tablelist->tableName));

//         if(tabItr==myCatalog->catalogTableHash.end())
//         {
//             cout<<"\n Error:Invalid table: "<<tablelist->tableName;
//             return false;
//         }
//         tablelist=tablelist->next;
//     }
//     //fill in the hash map
//     while(attsToSelectitr!=NULL)
//     {
//         queryattribs[string(attsToSelectitr->name)]=1;
//         attsToSelectitr=attsToSelectitr->next;
//     }
//     while(groupingAttsitr!=NULL)
//     {
//         queryattribs[string(groupingAttsitr->name)]=1;
//         groupingAttsitr=groupingAttsitr->next;

//     }
//     while(andList!=NULL)
//     {
//         struct OrList *orlist=andList->left;                
//         while(orlist!=NULL)
//         {
//             ComparisonOp *Op=orlist->left;
            
//             if(Op->left->code==4)
//             {
//                 queryattribs[string(Op->left->value)]=1;
//                 Split(Op->left->value);
                
//             }
//             if(Op->right->code==4)
//                 queryattribs[string(Op->right->value)]=1;
//             orlist=orlist->rightOr;

//         }
//         andList=andList->rightAnd;
//     }
//     //check for validity of all attribs in hashmap
//     map<string,int>::iterator queryattribsItr = queryattribs.begin();
//     while(queryattribsItr!=queryattribs.end())
//     {
//         if(!isAttValid(queryattribsItr->first))
//         {
//             cout<<"\n Error:Invalid Attribute "<<queryattribsItr->first;
//             return false;
//         }
//         queryattribsItr++;
//     }

//     //Checking grouping Semantics;
//     groupingAttsitr=groupingAtts;
//     attsToSelectitr=attsToSelect;
//     if(groupingAttsitr!=NULL)
//     {
//       while(attsToSelectitr!=NULL)
//       {
//         groupingAttsitr=groupingAtts;
//         while(groupingAttsitr!=NULL)
//         {
//             Split(string(attsToSelectitr->name));
//             string selAttr(attr);
//             string selalias(alias);
//             Split(string(groupingAttsitr->name));
//             string grpAttr(attr);
//             string grpalias(alias);
//             if(selalias.compare("")!=0 && grpalias.compare("")!=0)
//             {
//                 if(selalias.compare(grpalias)!=0)
//                 {
//                     cout<<"\n Error:Group must contain "<<attsToSelectitr->name;
//                     return false;
//                 }
//             }
//             if(selAttr.compare(grpAttr)==0)
//                 break;
//             groupingAttsitr=groupingAttsitr->next;
//         }
//         if(groupingAttsitr==NULL)
//         {
//             cout<<"\n Error:Group must contain "<<attsToSelectitr->name;
//             return false;
//         }
//        attsToSelectitr=attsToSelectitr->next;
//       }
//     }
    return true;
}