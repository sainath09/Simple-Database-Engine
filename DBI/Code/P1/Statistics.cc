#include "Statistics.h"
using namespace std;

// Here is a basic layout to get to our subset objects


// Send RelationName (its the key) to ->allsubSetMap -> value here is the subSetNum
// Send subSetNum (its the key) to -> subSetMap -> value we get here is structure named subSet
// Each of this subSet structure a map on attributes AttMap along with the other important values

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
	//Set the size of subset to increase when a new relation is added
	s.subSetNum = subSetsMap.size() + 1;
	//Set other factors for the subset struct
	s.numTuples = numTuples;
	s.relations.push_back(relName);
	//Map the subset by its num
	subSetsMap[s.subSetNum] =  s;
	//Map this subset number to the relation name that we want to add
	allsubSetMap[relName] = s.subSetNum;
	
}

void Statistics::AddAtt(char *relName, char *attName, long numDistincts)
{	
	//find existing relation if possible
	//Strategy being add to object push object to correct place, twist here being now  you are pushing attributes not relations like above
	//find the relation in global hashmap, that will provide us the subset num
	auto tempIt = allsubSetMap.find(relName);
	//convert values as required
	string tempRel (relName);
    string tempAttr(attName);
    int subSetNum = 0;
	tempRel += '.' + tempAttr;
	//check if end of map
	if (tempIt != allsubSetMap.end()){
		//extract the subset num for the relation
        subSetNum = tempIt->second;
		// in the subset map use the subset num to locate the structure object
		auto itSub = subSetsMap.find(subSetNum);
		if (itSub != subSetsMap.end()){
			//second value is the structure object
            subSet &tempS = itSub->second;		
			//add the name of attribute and the number of distinct values				
			tempS.AttMap[tempRel]  = numDistincts;
        } 
        // else cout << "ERROR!!!! in Retiriving subSet from map in AddAtt"<<endl;
						
	}
    // else cout<<"ERROR!!!! in Retiriving relation from map in AddAtt" << endl;
}

void Statistics::CopyRel(char *oldName, char *newName)
{
	//this follows the same concept to find the objects as described at beginning... we just copy everything we find
	// so start by finding subset num value of old one
	auto tempIt = allsubSetMap.find (oldName);
	if (tempIt != allsubSetMap.end()) {
		//now cause we are adding a new relation, +1
        int newsubSetNum = subSetsMap.size()+1;
		//create a new entry in allsubSetMap for the new name and map to the subset num
		allsubSetMap[newName]=  newsubSetNum;
		int oldsubSetNum = tempIt->second;
		//use the old subsetnum to pull up the structure
		auto itSub = subSetsMap.find(oldsubSetNum);
		if(itSub != subSetsMap.end()){
			//copy everything from the structure to the new one named tempsubset
            subSet &s = itSub->second;
			subSet tempsubSet;
			tempsubSet.subSetNum = newsubSetNum;
			tempsubSet.numTuples = s.numTuples;
			for (auto it =  s.AttMap.begin();it!= s.AttMap.end();it++) {
				// using for loop otherwise it wont do deep copy of the attribute hashmap.
				 int position = it->first.find('.');
				 string tempAtt(newName);
				 tempAtt += '.' + it->first.substr(position+1);
				 //deep copy to the "newName.attributename"
				 tempsubSet.AttMap[tempAtt] = it->second;
			}
			tempsubSet.relations.push_back(newName);
			//add all the data you copied back to the subset map
			subSetsMap[newsubSetNum] = tempsubSet;
			
		}
        // else cout << "ERROR!!!! in Retiriving subSet from map in copyRel"<<endl;
		
	}
    // else cout << "ERROR!!!! in Retiriving Relation from map in copyRel"<<endl;
}
	
void Statistics::Read(const char *fromWhere){	
	//read the write function first, this would make more sense then
	string line;
	ifstream in(fromWhere);
	// first line has the total number of subsets 
	getline(in,line); 
	int numsubSets = stoi(line);
	//now get a mapping for allsubSetMap and read the subsetNums and the related relationship names
	getline(in,line);
	istringstream iss(line);
	vector<string> temp;
	//Elegant way of doing .split found on stackoverflow. C++ should really be able to do string split. this is splitting on every space
	copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter(temp));
	// 	all the values are in array temp, even values and zero being relnames and odd values being subsetnums, map these correctly
	for (int i=0;i<temp.size(); i+=2) allsubSetMap[temp[i]] = stoi(temp[i+1], NULL);
	//now comes all the subSetMap mappings, each of the subSetMap element is in a different line
	while (getline(in,line)){
		istringstream iss(line);
		vector<string> temp;
		//again split on space using the method above, but now the values are stored a bit differently
		
	
		copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter(temp));
		// first two are the properties of subSetMap element
		int subSetNumber = stoi(temp[0], NULL);
		long numTuples = stoi(temp[1], NULL);
		// rest are all the mappings in AttMap map
		map<string,long> attr;
		//so run a for loop from 3rd value and read odd elements of temp as key and even elements as the value till the end of line
		for (int i=2;i<temp.size(); i+=2) {
			attr[temp[i]] = stoi(temp[i+1], NULL);
		}
		//add it all back to the subSetMap object
		subSet tempS;
		tempS.subSetNum = subSetNumber;
		tempS.numTuples = numTuples;
		tempS.AttMap = attr;
		subSetsMap[subSetNumber] =  tempS; 
	}
	//Done!!
}
void Statistics::Write(const char *fromWhere){
	//create a writer
	ofstream out(fromWhere);
	//write out number of all subsets present currently
	out << subSetsMap.size() << endl;
	//now goto to each map entry and print the name of relation and then its subset num.
	//print all such mappings in a single line, we will read this to create back allsubSetMap to do faster lookups of relationnames
	for (auto it = allsubSetMap.begin();it!=allsubSetMap.end();it++) {
		out << it->first <<" ";
		auto tempIt = allsubSetMap.find(it->first);
		out << tempIt->second <<" ";
	}
	out << endl;
	//now for each subsetNum mentioned above, go and recursively print everything inside. Use a loop for deep reading AttMap
	for (auto it = subSetsMap.begin();it != subSetsMap.end();it++){
		//this is the subSetNum
		out << it->first << " ";
		auto itSub = subSetsMap.find(it->first);
		//now print the other properties of this subset
		out << itSub->second.numTuples << " ";
		//finally deep print all the attribute properties
		for (auto ott =  itSub->second.AttMap.begin();ott !=  itSub->second.AttMap.end();ott++){
			out << ott->first << " " << ott->second << " ";
		}
		//goto next line when one subsetNum is done
		out << endl;
	}
}

void  Statistics::Apply(struct AndList *parseTree, char *relNames[], int numToJoin)
{
	//make sure we can calculate the estimate
	auto estimate = calcEstimate(parseTree);
	// if not just end on error
	if (estimate.first.first == -1){
		cout << "ERROR!!!! Input is not correct. got emtpy estimate from calcEstimate in apply!" << endl;		
		return;
	}
	//second set of values returned, capture them 
	int op1 = estimate.second[0];
	int op2 = estimate.second[1];
	if (op1 != -1 && op2 == -1) {
		//if first one exists and second doesnt,find the first 
		//and calculate the resultant total selection factor * no of rows. 
		auto itSub = subSetsMap.find(op1);
		if (itSub != subSetsMap.end()) {
			itSub->second.numTuples = estimate.first.first*estimate.first.second;
		} 
		else cout<<"ERROR!!! cannot find subset in apply"<<endl;
		return;
	} 
	else if (op1 == -1 && op2 == -1) {
		//handle corner cases
		cout << "ERROR!!! calcestimate did not return correctly in apply" << endl;
		return;
	}
	//normallly find both, first
	auto itSub1 = subSetsMap.find(op1);
	if (itSub1 == subSetsMap.end()){
		cout << "Error!!!! cannot get subset 1 in apply" << endl;
		return;
	}	

	//...and second
	auto itSub2 = subSetsMap.find(op2);
	if (itSub2 == subSetsMap.end()){
		cout << "Error!!!! cannot get subset 2 in apply" << endl;
		return;
	}	
	//now look at the subset structure of both of these pairs
	subSet &s1 = itSub1->second;
	subSet &s2 = itSub2->second;
	
	if (s1.subSetNum == s2.subSetNum) { //again handle the stupid corner case error should have same subset num .. dont compare to urself
	 cout<<"Should never happen" << endl; return;}
	
	
	//calculate total rows like we did in estimate
	s1.numTuples = estimate.first.first*estimate.first.second;


	for(int i=0;i<s2.relations.size();i++){
		// copy all values out of s2 for allsubSetMap and set them now as belonging to s1
		allsubSetMap.erase(s2.relations[i]);
		allsubSetMap[s2.relations[i]]= s1.subSetNum;
	}

	for (auto it = s1.AttMap.begin(); it!=s1.AttMap.end(); it++) {
		// update the number of rows to the multiplied values in s1
		s1.AttMap[it->first] = it->second*estimate.first.second;

	}

	for (auto it = s2.AttMap.begin(); it!=s2.AttMap.end(); it++) {
		//add values from s2 to s1 , yes s1 ... cause we want to join no
		s1.AttMap[it->first] = it->second*estimate.first.second;
	}
	
	
	for(int i=0;i<s2.relations.size();i++){
		//push all the s2 data to s1
		s1.relations.push_back(s2.relations[i]);
	}
	
	//delete s2 now
	subSetsMap.erase(s2.subSetNum);
	
	
}
double Statistics::Estimate(struct AndList *parseTree, char **relNames, int numToJoin){
	//the functionality of estimate and apply is so similar, we made a method for the common parts
	//pass all the data to this function
	auto estimate = calcEstimate(parseTree);
	//return the resultant total selection factor * no of rows.
	//we dont really need the second pair of data returned from the  calc function for just this estimate function we will use it in apply
	return estimate.first.first*estimate.first.second;
}

attStats Statistics::getattStats(string attr){

	//just a simple deep copy of AttMap, for a particular attr 
	attStats stats;
	//across subsetmap elements, look for attr ... 
	//it would be a combo of relationname and attribute name and thus should be unique
	for (auto itSub =  subSetsMap.begin();itSub != subSetsMap.end();itSub++){
		for (auto it = itSub->second.AttMap.begin();it!=itSub->second.AttMap.end();it++){
			if (it->first.compare(attr)==0){
			//copy all the data if found	
				stats.subSetNum = itSub->first;
				stats.numTuples = itSub->second.numTuples;
				stats.numDistinct = it->second;
				return stats;	
			}
		}
	}	
	//otherwise return -1
	stats.subSetNum = -1;
	stats.numTuples = -1;
	stats.numDistinct = -1;
	return stats;
}

void Statistics::adjustSelectivityFactor(string attName, vector<string> &eachAtts, double& eachSelFactor, int code){
	//find the AttMap for attName and deep copy it to attstats
	auto attstats = getattStats(attName);
	double  sel = 1.0;
	if(code == EQUALS){
		//you will select based on num of distinct values if its equals
		sel = 1.0/attstats.numDistinct;
	}
	else sel = 1.0/3.0;  // ... just do a 3rd
	if (eachAtts.empty()) {
		//if eachAtts is not available, directly push that we will be selecting a 3rd
		eachSelFactor = sel;
		eachAtts.push_back(attName);
	} 
	else if (find(eachAtts.begin(), eachAtts.end(), attName) == eachAtts.end()) {
		//otherwise calculate exactly how many select 
		eachSelFactor = 1 - (1 - eachSelFactor) * (1 - sel);
		eachAtts.push_back(attName);
	} 
	else eachSelFactor += sel;
	
	//be careful cause final value is returned directly in eachSelFactor , no need to return value, but it shouldnt overflow or do weird stuff
	return ;
}

//returns a weird data structure, may make it more elegant in next assignment
pair<pair<long , double>, vector<int> > Statistics::calcEstimate(const struct AndList *andList) {
	//just declare some variables we will use later, -1 means all
	long  numOfRows;
	double totSelFactor = 1.0;

	int subSetNum1 = -1;
	int subSetNum2 = -1;
	int selsubSetNum1 = -1;
	int selsubSetNum2 = -1;
	bool FLAG = false;
	long double tuplesToSelect = 0;
	// check the andlist for members
	while (andList) {
		// find the orList members 
		struct OrList *innerOrList = andList->left;
		//defaults
		double eachSelFactor = 1.0;
		vector<string> eachAtts;  
		//now while there are ors left
		while (innerOrList) {
			//traverse and find a compare between names
			if (innerOrList->left->left->code == NAME && innerOrList->left->right->code == NAME) {
				//Only consider if the compare is equal type
				if(innerOrList->left->code == EQUALS){
					//check if the attribute even exists, if so copy that attributes AttMap here
					auto lStats = getattStats(innerOrList->left->left->value);
					auto rStats = getattStats(innerOrList->left->right->value);
					FLAG = true;
					if (lStats.subSetNum == -1 || rStats.subSetNum==-1){
						//if they dont exist just exit
						//cout<<"Error: not able to find attribute name in any subSets" << endl;
						subSetNum1 = -1;
						subSetNum2 = -1;
						vector<int> ret;
						ret.push_back(subSetNum1);
						ret.push_back(subSetNum2);

						return make_pair(make_pair(numOfRows, totSelFactor), ret);
					}
					//otherwise find the number of rows in each
					long lRows = lStats.numTuples;
					long rRows = rStats.numTuples;
					//find which has more rows
					int maxRows =  max(lStats.numDistinct, rStats.numDistinct);
					//multiple rows as a join would and divide by max rows value
					numOfRows = (double) (((double) (lRows * rRows)) / (double) maxRows);

					subSetNum1 = lStats.subSetNum;
					subSetNum2 = rStats.subSetNum;
				}
				else {
					//this should even come here, dont check unequal ones
					cout<<"ERROR!!!! can only be done with equals. Should never come here in calcEstimate" <<endl;
					vector<int> ret;
					ret.push_back(-1);
					ret.push_back(-1);
					return make_pair(make_pair(-1,-1), ret);

				}

			} 
			else if (innerOrList->left->left->code == NAME ) {
				//now if one is name and other isnt, adjust percentage of row selection
				adjustSelectivityFactor(innerOrList->left->left->value, eachAtts, eachSelFactor, innerOrList->left->code);
				//get AttMap for the left one
				auto lStats = getattStats(innerOrList->left->left->value);
				tuplesToSelect = lStats.numTuples;
				selsubSetNum1 = lStats.subSetNum;

			}
			else if (innerOrList->left->right->code == NAME) {
				//now if one is name and other isnt, adjust percentage of row selection
				adjustSelectivityFactor(innerOrList->left->right->value, eachAtts, eachSelFactor, innerOrList->left->code);
				//get AttMap for the right one
				auto rStats = getattStats(innerOrList->left->right->value);
				tuplesToSelect = rStats.numTuples;
				selsubSetNum2 = rStats.subSetNum;
			}
		//keep traversing forward in the or tree
		innerOrList = innerOrList->rightOr;
		}
		//reset
		eachAtts.clear();
		//pick the value returned to eachSelFactor
		totSelFactor *= eachSelFactor;
		//keep traversing forward in the and tree
		andList = andList->rightAnd;
		
	}
	
	if(!FLAG){
		//only goes here if one of the side is name and other isnt
		numOfRows = tuplesToSelect;
		subSetNum1 = selsubSetNum1;
		subSetNum2 = selsubSetNum2;
	}
	//set up all the values correctly and return
	vector<int> ret;
	ret.push_back(subSetNum1);
	ret.push_back(subSetNum2);
	return make_pair(make_pair(numOfRows, totSelFactor), ret);
	
}
