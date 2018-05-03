#ifndef CATALOG_H
#define	CATALOG_H

#include <string>
#include <map>
#include <vector>
#include "Statistics.h"

using namespace std;

//class for attributes and corressponding types... will combine into it a linked list of tables where they occur
class attrType
{
public:
    string attr;
    string type;

attrType(string a,string b)
    {
    attr = a;
    type = b;
    }
};


//Linked list of tables name for a particular att .. this one is supposed to attach to the class abov using unordered_ hashmaps
class TblLnkList
{

public:
    TblLnkList *next;
    string tableName;

    TblLnkList(string a)
    {
        tableName = a;
        next=NULL;
    }

};



class Catalog
{
private:
    
    Catalog();
    
    //initializing catalog class
public:
    static Catalog *cat;
    map<string,vector<attrType*> > relToAttr;
    map<string,TblLnkList*> attrToTable;
    map<string,string> tableToFile;
    Statistics *stats;
    Catalog * get();
    void  init();
    static Catalog* instantiate();


      
};

#endif