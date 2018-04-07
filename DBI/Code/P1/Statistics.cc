#include "Statistics.h"
using namespace std;

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
	subSet s;
	s.subSetNum = subSetsMap.size() + 1;
	s.numTuples = numTuples;
	s.relations.push_back(relName);
	subSetsMap[s.subSetNum] =  s;
	allsubSetMap[relName] = s.subSetNum;
	
}

void Statistics::AddAtt(char *relName, char *attName, long numDistincts)
{	
	auto tempIt = allsubSetMap.find(relName);
	string tempRel (relName);
    string tempAttr(attName);
    int subSetNum = 0;
	tempRel += '.' + tempAttr;
	if (tempIt != allsubSetMap.end()){
        subSetNum = tempIt->second;
		auto itSub = subSetsMap.find(subSetNum);
		if (itSub != subSetsMap.end()){
            subSet &tempS = itSub->second;						
			tempS.AttMap[tempRel]  = numDistincts;
        } 
        else cout << "ERROR!!!! in Retiriving subSet from map in AddAtt"<<endl;
						
	}
    else cout<<"ERROR!!!! in Retiriving relation from map in AddAtt" << endl;
}

void Statistics::CopyRel(char *oldName, char *newName)
{
	auto tempIt = allsubSetMap.find (oldName);
	if (tempIt != allsubSetMap.end()) {
        int newsubSetNum = subSetsMap.size()+1;
		allsubSetMap[newName]=  newsubSetNum;
		int oldsubSetNum = tempIt->second;
		auto itSub = subSetsMap.find(oldsubSetNum);
		if(itSub != subSetsMap.end()){
            subSet &s = itSub->second;
			subSet tempsubSet;
			tempsubSet.subSetNum = newsubSetNum;
			tempsubSet.numTuples = s.numTuples;
			for (auto it =  s.AttMap.begin();it!= s.AttMap.end();it++) {
				 int position = it->first.find('.');
				 string tempAtt(newName);
				 tempAtt += '.' + it->first.substr(position+1);
				 tempsubSet.AttMap[tempAtt] = it->second;
			}
			tempsubSet.relations.push_back(newName);

			subSetsMap[newsubSetNum] = tempsubSet;
			
		}
        else cout << "ERROR!!!! in Retiriving subSet from map in copyRel"<<endl;
		
	}
    else cout << "ERROR!!!! in Retiriving Relation from map in copyRel"<<endl;
}
	
void Statistics::Read(const char *fromWhere)
{	string line;
	ifstream in(fromWhere);
	getline(in,line); // number of subsets in first line
	int numsubSets = stoi(line);
	getline(in,line);
	istringstream iss(line);
	vector<string> temp;
	//Elegant way of doing .split found on stackoverflow. C++ should really be able to do string split.
	copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter(temp));
	for (int i=0;i<temp.size(); i+=2) allsubSetMap[temp[i]] = stoi(temp[i+1], NULL);

	while (getline(in,line)){
		istringstream iss(line);
		vector<string> temp;
		copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter(temp));
		int subSetNumber = stoi(temp[0], NULL);
		long numTuples = stoi(temp[1], NULL);
		unordered_map<string,long> attr;
		for (int i=2;i<temp.size(); i+=2) {
			attr[temp[i]] = stoi(temp[i+1], NULL);
		}
		subSet tempS;
		tempS.subSetNum = subSetNumber;
		tempS.numTuples = numTuples;
		tempS.AttMap = attr;
		subSetsMap[subSetNumber] =  tempS; 
	}
	
}
void Statistics::Write(const char *fromWhere)
{
	ofstream out(fromWhere);
	out << subSetsMap.size() << endl;
	for (auto it = allsubSetMap.begin();it!=allsubSetMap.end();it++) {
		out << it->first <<" ";
		auto tempIt = allsubSetMap.find(it->first);
		out << tempIt->second <<" ";
	}
	out << endl;
	for (auto it = subSetsMap.begin();it != subSetsMap.end();it++){
		out << it->first << " ";
		auto itSub = subSetsMap.find(it->first);
		out << itSub->second.numTuples << " ";
		for (auto ott =  itSub->second.AttMap.begin();ott !=  itSub->second.AttMap.end();ott++){
			out << ott->first << " " << ott->second << " ";
		}
		out << endl;
	}
}

void  Statistics::Apply(struct AndList *parseTree, char *relNames[], int numToJoin)
{
	auto estimate = calcEstimate(parseTree, relNames,  numToJoin);
	if (estimate.first.first == -1){
		cout << "ERROR!!!! Input is not correct. got emtpy estimate from calcEstimate in apply!" << endl;		
		return;
	}
	
	int op1 = estimate.second[0];
	int op2 = estimate.second[1];
	if (op1 != -1 && op2 == -1) {
		auto itSub = subSetsMap.find(op1);
		if (itSub != subSetsMap.end()) {
			itSub->second.numTuples = estimate.first.first*estimate.first.second;
		} 
		else cout<<"ERROR!!! cannot find subset in apply"<<endl;
		return;
	} 
	else if (op1 == -1 && op2 == -1) {
		cout << "ERROR!!! calcestimate did not return correctly in apply" << endl;
		return;
	}

	auto itSub1 = subSetsMap.find(op1);
	if (itSub1 == subSetsMap.end()){
		cout << "Error!!!! cannot get subset 1 in apply" << endl;
		return;
	}	
	auto itSub2 = subSetsMap.find(op2);
	if (itSub2 == subSetsMap.end()){
		cout << "Error!!!! cannot get subset 2 in apply" << endl;
		return;
	}	
	
	subSet &s1 = itSub1->second;
	subSet &s2 = itSub2->second;
	
	if (s1.subSetNum == s2.subSetNum) { cout<<"Should never happen" << endl; return;}

	s1.numTuples = estimate.first.first*estimate.first.second;


	for(int i=0;i<s2.relations.size();i++){

		allsubSetMap.erase(s2.relations[i]);
		allsubSetMap[s2.relations[i]]= s1.subSetNum;
	}

	for (auto it = s1.AttMap.begin(); it!=s1.AttMap.end(); it++) {
		s1.AttMap[it->first] = it->second*estimate.first.second;

	}

	for (auto it = s2.AttMap.begin(); it!=s2.AttMap.end(); it++) {
		s1.AttMap[it->first] = it->second*estimate.first.second;
	}
	
	
	for(int i=0;i<s2.relations.size();i++){
		s1.relations.push_back(s2.relations[i]);
	}
	

	subSetsMap.erase(s2.subSetNum);
	
	
}
double Statistics::Estimate(struct AndList *parseTree, char **relNames, int numToJoin){
	auto estimate = calcEstimate(parseTree, relNames, numToJoin );
	return estimate.first.first*estimate.first.second;
}

attStats Statistics::getattStats(string attr, char **relNames, int numToJoin){
	attStats stats;
	for (auto itSub =  subSetsMap.begin();itSub != subSetsMap.end();itSub++){
		for (auto it = itSub->second.AttMap.begin();it!=itSub->second.AttMap.end();it++){
			if (it->first.compare(attr)==0){
				stats.subSetNum = itSub->first;
				stats.numTuples = itSub->second.numTuples;
				stats.numDistinct = it->second;
				return stats;	
			}
		}
	}	
	stats.subSetNum = -1;
	stats.numTuples = -1;
	stats.numDistinct = -1;
	return stats;
}

void Statistics::adjustSelectivityFactor(string attName, vector<string> &eachAtts, double& eachSelFactor, int code, char **relNames, int numToJoin){
	
	auto attstats = getattStats(attName, relNames, numToJoin);
	double  sel = 1.0;
	if(code == EQUALS){
		sel = 1.0/attstats.numDistinct;
	}
	else sel = 1.0/3.0;
	if (eachAtts.empty()) {
		eachSelFactor = sel;
		eachAtts.push_back(attName);
	} 
	else if (find(eachAtts.begin(), eachAtts.end(), attName) == eachAtts.end()) {
		eachSelFactor = 1 - (1 - eachSelFactor) * (1 - sel);
		eachAtts.push_back(attName);
	} 
	else eachSelFactor += sel;
	return ;
}


pair<pair<long , double>, vector<int> > Statistics::calcEstimate(const struct AndList *andList, char **relNames, int numToJoin) {
	
	long  numOfRows;
	double totSelFactor = 1.0;

	int subSetNum1 = -1;
	int subSetNum2 = -1;
	int selsubSetNum1 = -1;
	int selsubSetNum2 = -1;
	bool FLAG = false;
	long double tuplesToSelect = 0;
	while (andList) {
		struct OrList *innerOrList = andList->left;
		double eachSelFactor = 1.0;
		vector<string> eachAtts;  
		while (innerOrList) {
			if (innerOrList->left->left->code == NAME && innerOrList->left->right->code == NAME) {
				if(innerOrList->left->code == EQUALS){

					
					auto lStats = getattStats(innerOrList->left->left->value, relNames, numToJoin);
					auto rStats = getattStats(innerOrList->left->right->value, relNames, numToJoin);
					FLAG = true;
					if (lStats.subSetNum == -1 || rStats.subSetNum==-1){
						cout<<"Error: not able to find attribute name in any subSets" << endl;
						subSetNum1 = -1;
						subSetNum2 = -1;
						vector<int> ret;
						ret.push_back(subSetNum1);
						ret.push_back(subSetNum2);

						return make_pair(make_pair(numOfRows, totSelFactor), ret);
					}
					long lRows = lStats.numTuples;
					long rRows = rStats.numTuples;
					int maxRows =  max(lStats.numDistinct, rStats.numDistinct);
					numOfRows = (double) (((double) (lRows * rRows)) / (double) maxRows);

					subSetNum1 = lStats.subSetNum;
					subSetNum2 = rStats.subSetNum;
				}
				else {
					cout<<"ERROR!!!! can only be done with equals. Should never come here in calcEstimate" <<endl;
					vector<int> ret;
					ret.push_back(-1);
					ret.push_back(-1);
					return make_pair(make_pair(-1,-1), ret);

				}

			} 
			else if (innerOrList->left->left->code == NAME ) {
				adjustSelectivityFactor(innerOrList->left->left->value, eachAtts, eachSelFactor, innerOrList->left->code, relNames, numToJoin);
				auto lStats = getattStats(innerOrList->left->left->value, relNames, numToJoin);
				tuplesToSelect = lStats.numTuples;
				selsubSetNum1 = lStats.subSetNum;

			}
			else if (innerOrList->left->right->code == NAME) {
				adjustSelectivityFactor(innerOrList->left->right->value, eachAtts, eachSelFactor, innerOrList->left->code, relNames, numToJoin);
				auto rStats = getattStats(innerOrList->left->right->value, relNames, numToJoin);
				tuplesToSelect = rStats.numTuples;
				selsubSetNum2 = rStats.subSetNum;
			}
		innerOrList = innerOrList->rightOr;
		}
		eachAtts.clear();
		totSelFactor *= eachSelFactor;
		andList = andList->rightAnd;
		
	}
	
	if(!FLAG){
		numOfRows = tuplesToSelect;
		subSetNum1 = selsubSetNum1;
		subSetNum2 = selsubSetNum2;
	}
	vector<int> ret;
	ret.push_back(subSetNum1);
	ret.push_back(subSetNum2);
	return make_pair(make_pair(numOfRows, totSelFactor), ret);
	
}
