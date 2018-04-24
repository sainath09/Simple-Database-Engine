#include "Catalog.h"
#include "Statistics.h"


Catalog::Catalog(){
    init();
    stats = new Statistics();
    stats->Read("Statistics.txt");
} 

Catalog* Catalog::cat = NULL;
Catalog* Catalog::instantiate(){
    if(cat==NULL)
    {
        cat=new Catalog();
    }
    return cat;
}
extern string gl_cat;
void Catalog::init(){
 
    ifstream in(gl_cat.c_str()); 
    string readRel;
    string readLine;
    string readType;

    while(getline(in,readLine))
    {
        if(readLine == "") continue;
        if(readLine=="BEGIN")
        {   
            //next line has relation name
            getline(in,readRel);  
            //and then the relations file name
            getline(in,readLine);   
            //get those and map them to a hash for any later use
            tableToFile[readRel]=readLine;
            
        }else if(readLine!="END"){
            //define the variables as needed
            stringstream ss(readLine);
            attrType *atType;
            //1st line that is attribute is alread in readline, fetch second one
            ss>>readLine;
            ss>>readType;
           // getline(in,readType);
            //create an entry in attribute type
            atType =new attrType(readLine,readType);
            relToAttr[readRel].push_back(atType);

            auto it=attrToTable.find(readLine);

            if(it!=attrToTable.end()){
                //add relname to existing linked list if available for that attribute
                TblLnkList *link =new TblLnkList(readRel);
                link->next=attrToTable[readLine];
                attrToTable[readLine]=link;
            }else{
                //create new link list struct if it doesnt exist yet
                TblLnkList *link =new TblLnkList(readRel);
                attrToTable[readLine]=link;

            }
        }
        //In case its anything else like END just run past it   
    }
}