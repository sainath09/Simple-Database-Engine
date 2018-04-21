#include "Catalog.h"


//TODO: See if we can instance without this
Catalog* Catalog::instantiate(){
    if(cat==NULL)
    {
        cat=new Catalog();
    }
    return cat;
}

void Catalog::init(){
 
    ifstream in("catalog"); //FIXME: Use gl_cat here instead
    string readRel;
    string readLine;
    string readType;

    while(getline(in,readLine))
    {
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
            attrType *atType;
            //1st line that is attribute is alread in readline, fetch second one
            getline(in,readType);
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