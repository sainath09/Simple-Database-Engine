#ifndef STATISTICS_
#define STATISTICS_

#include "ParseTree.h"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>


using namespace std;
typedef struct attStats{
	int subSetNum;
	long numTuples;
	long numDistinct;
}attStats;
typedef struct subSet{
	int subSetNum;
	unordered_map<string,long> AttMap;
	long numTuples;
	vector<string> relations;
}subSet;

class Statistics
{
private:
	unordered_map<int, subSet> subSetsMap;
	unordered_map<string, int> allsubSetMap;
public:
	Statistics();
	Statistics(Statistics &copyMe);
	~Statistics();
	//Uses UO Maps
	void AddRel(char *relName, int numTuples);
	void AddAtt(char *relName, char *attName, long numDistincts);
	void CopyRel(char *oldName, char *newName);
	//Read write Functions
	void Read(const char *fromWhere);
	void Write(const char *fromWhere);
	
	void  Apply(struct AndList *parseTree, char *relNames[], int numToJoin);
	double Estimate(struct AndList *parseTree, char **relNames, int numToJoin);

	attStats getattStats(string attr, char **relNames, int numToJoin);
	void adjustSelectivityFactor(string attName, vector<string> &eachAtts, double& eachSelFactor, int code, char **relNames, int numToJoin);
	pair<pair<long, double>, vector<int> > calcEstimate(const struct AndList *andList, char **relNames, int numToJoin);

};

#endif
