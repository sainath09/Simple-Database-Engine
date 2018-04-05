#include "Statistics.h"

Statistics::Statistics()
{

}
Statistics::Statistics(Statistics &copyMe)
{
}
Statistics::~Statistics()
{
}

void Statistics::AddRel(char *relName, int numTuples)
{
    structRel tempRel;
    tempRel.numTuples = numTuples;
    tempRel.numRel = 1;
    string relname(relName);
    mapRel[relname]=tempRel;
}
void Statistics::AddAtt(char *relName, char *attName, int numDistincts)
{
    string tempRel(relName);
    string tempAttr(attName);
    if(numDistincts == -1) numDistincts = mapRel[relName].numTuples;
    mapRel[relName].mapAttr[tempAttr] = numDistincts;
}
void Statistics::CopyRel(char *oldName, char *newName)
{
    string oldname(oldName);
    string newname(newName);
    
    structRel toCopy;
    toCopy.numTuples = mapRel[oldname].numTuples;
    toCopy.numRel = mapRel[oldname].numRel;
    for(auto it_attr = mapRel[oldname].mapAttr.begin();it_attr != mapRel[oldname].mapAttr.end();it_attr++ ){

        toCopy.mapAttr.insert(make_pair(it_attr->first,it_attr->second));
    }
    mapRel[newname] = toCopy;
}
	
void Statistics::Read(char *fromWhere){
    ifstream in(fromWhere);
    if(!in.is_open()){
        cerr<<"No file to read Please check the file path or that file exists!";
        exit(1);
    }
    string line;
    getline(in,line); //Dumping out startOfRelation here
    while(getline(in,line)){
        if(line == "RelationName:"){
            string relname;
            in>>relname;
            in>>line; //Dumping numberOfRelations to line
            structRel tempRel;
            in>>tempRel.numRel;
            in>>line; // Dumping Attrs to line
            in>>tempRel.numTuples;
            while(getline(in,line) && line != "startOfRelation" && line !="EOF"){
                string attrName;
                 attrName = line;
                in>>tempRel.mapAttr[attrName];
            }
            mapRel[relname] = tempRel;

        }


    }

}
void Statistics::Write(char *fromWhere){
    ofstream out(fromWhere);
    for(auto it = mapRel.begin();it!= mapRel.end();it++){
        out<<"startOfRelation\n";
        out<<"RelationName:\n"<<it->first<<"\n";
        out<<"numberOfRelations:\n"<<it->second.numRel<<"\n";
        out<<"Attrs:\n"<<it->second.numTuples<<"\n";
        for(auto it_attr = it->second.mapAttr.begin();it_attr != it->second.mapAttr.end();it_attr++){
            out<<it_attr->first<<"\n";
            out<<it_attr->second<<"\n";
        }
    }
    out<<"EOF";
    out.close();
}

void  Statistics::Apply(struct AndList *parseTree, char *relNames[], int numToJoin)
{
}
double Statistics::Estimate(struct AndList *parseTree, char **relNames, int numToJoin)
{
}

