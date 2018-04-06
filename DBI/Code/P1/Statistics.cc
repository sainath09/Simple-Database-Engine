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

        toCopy.mapAttr[it_attr->first] = it_attr->second;
    }
    mapRel[newname] = toCopy;

    // cout<<"BEGIN PRINT"<<endl;
    // for(auto it = mapRel[newname].mapAttr.begin();it!= mapRel[newname].mapAttr.end();it++){
    //     cout<<it->first<<endl;
    //     cout<<it->second<<endl;
    // }
    // cout<<"end PRINT"<<endl;
}
	
void Statistics::Read(char *fromWhere){
    mapRel.clear();
    resFromEstimate = 0.0;
    ifstream in(fromWhere);
    if(!in.is_open()){
        cerr<<"No file to read Please check the file path or that file exists!";
        exit(1);
    }
    string line;
    getline(in,line); //Dumping out startOfRelation here
    
    while(getline(in,line)){
       // cout<<"Relation name"<<line<<endl;
        if(line == "RelationName:"){
            string relname;
            in>>relname;
            in>>line; //Dumping numberOfRelations to line
            structRel tempRel;
            in>>tempRel.numRel;
            in>>line; // Dumping Attrs to line
            in>>tempRel.numTuples;
            getline(in,line);
            
            while(getline(in,line) && line != "startOfRelation" && line !="EOF"){
               // cout<<"line:"<<line<<endl;
                string attrName;
                 attrName = line;
                in>>tempRel.mapAttr[attrName];
                getline(in,line);
            }
            mapRel[relname] = tempRel;

        }
    }
    // cout<<"BEGIN PRINT"<<endl;
    // for(auto it = mapRel.begin();it!= mapRel.end();it++){
    //     cout<<it->first<<endl;
    //     cout<<it->second.numTuples<<endl;
    //     cout<<it->second.numRel<<endl;
    //     for(auto itt = it->second.mapAttr.begin();itt!= it->second.mapAttr.end();itt++ ){
    //         cout<<itt->first<<":"<<itt->second<<endl;
    //     }
    // }
    // cout<<"end PRINT"<<endl;

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

void  Statistics::Apply(struct AndList *parseTree, char *relNames[], int numToJoin){
    
    struct OrList *innerOrList;
    struct AndList *tempParser = parseTree;
    while(tempParser){
        if(parseTree->left){
            innerOrList = tempParser->left;
            while(innerOrList){
                if(innerOrList->left->left->code == 4 && innerOrList->left->right->code == 4){ //FIXME: 4 is for string and 3 is for name
                    unordered_map<string,int>::iterator attIt[2];
                    unordered_map<string,structRel>::iterator relIt[2];
                    structRel tempjoinRelation;
                    for(auto it = mapRel.begin();it!= mapRel.end();it++ ){
                        attIt[0] = it->second.mapAttr.find(innerOrList->left->left->value);
                        if(attIt[0] != it->second.mapAttr.end()){
                            relIt[0] = it;
                            break;
                        }
                    }
                    for(auto it = mapRel.begin();it!= mapRel.end();it++ ){
                        attIt[1] = it->second.mapAttr.find(innerOrList->left->right->value);
                        if(attIt[1] != it->second.mapAttr.end()){
                            relIt[1] = it;
                            break;
                        }
                    }
                    string RelationName = relIt[0]->first +"|"+ relIt[1]->first;
                    tempjoinRelation.numTuples = resFromEstimate;
                    tempjoinRelation.numRel = numToJoin;
                    for(int i = 0;i<2;i++){
                        for(auto it = relIt[i]->second.mapAttr.begin();it!= relIt[i]->second.mapAttr.end();it++){
                            tempjoinRelation.mapAttr.insert(*it);
                        }
                        mapRel.erase(relIt[i]);
                    }
                    mapRel.insert({RelationName,tempjoinRelation});
                }
                else{
                    unordered_map<string,int>::iterator attIt;
                    unordered_map<string,structRel>::iterator relIt;
                    for(auto it = mapRel.begin();it!=mapRel.end();it++){
                        attIt = it->second.mapAttr.find(innerOrList->left->left->value);
                        if(attIt != it->second.mapAttr.end()){
                            relIt = it;
                            break;
                        }
                    }
                    relIt->second.numTuples = resFromEstimate;
                }
                innerOrList = innerOrList->rightOr;
            }
        }
         tempParser = tempParser->rightAnd;
    }
}
double Statistics::Estimate(struct AndList *parseTree, char **relNames, int numToJoin){
    struct AndList* tempParser = parseTree;
    struct OrList* innerOrList;
    double res = 0.0,frac = 1.0;
    int FLAG = 0;
    if(!tempParser){
        if(numToJoin>1) return -1;
        return mapRel[relNames[0]].numTuples;
    }
    while(tempParser){
        if(tempParser->left){
            innerOrList = tempParser->left;
            double orFrac = 0.0;
            unordered_map<string,int>::iterator attItEnd;
            while(innerOrList){
                if(innerOrList->left->left->code == 4 && innerOrList->left->right->code == 4){ //FIXME: 4 is for string and 3 is for name
                    unordered_map<string,int>::iterator attIt[2];
                    unordered_map<string,structRel>::iterator relIt[2];
                    structRel tempjoinRelation;
                    for(auto it = mapRel.begin();it!= mapRel.end();it++ ){
                        attIt[0] = it->second.mapAttr.find(innerOrList->left->left->value);
                        if(attIt[0] != it->second.mapAttr.end()){
                            relIt[0] = it;
                            break;
                        }
                    }
                    for(auto it = mapRel.begin();it!= mapRel.end();it++ ){
                        attIt[1] = it->second.mapAttr.find(innerOrList->left->right->value);
                        if(attIt[1] != it->second.mapAttr.end()){
                            relIt[1] = it;
                            break;
                        }
                    }
                    double maxi = (double)max(attIt[0]->second,attIt[1]->second);
                    if(FLAG == 0)
                        res =  (double)relIt[0]->second.numTuples*(double)relIt[1]->second.numTuples/maxi;
                    else
                        res *= (double)relIt[1]->second.numTuples/maxi;
                    FLAG = 1;
                }
                else{
                    unordered_map<string,int>::iterator attIt;
                    unordered_map<string,structRel>::iterator relIt;
                    for(auto it = mapRel.begin();it!=mapRel.end();it++){
                        attIt = it->second.mapAttr.find(innerOrList->left->left->value);
                        if(attIt != it->second.mapAttr.end()){
                            relIt = it;
                            break;
                        }
                    }
                    if(res == 0.0) res = ((double)relIt->second.numTuples);
                    double tempf;
                    if(innerOrList->left->code == 3) tempf = 1.0/attIt->second; //FIXME: value of equals
                    else tempf = 1.0/3.0;
                    if(attIt != attItEnd)
                        orFrac = tempf + orFrac - (tempf*orFrac);
                    else  
                        orFrac += tempf;
                    attItEnd = attIt;
                }
                innerOrList = innerOrList->rightOr;
            }
            if(orFrac != 0.0) frac *= orFrac;
        }
        tempParser = tempParser->rightAnd;       
    }
    res *= frac;
    resFromEstimate = res;
    return res;
}

