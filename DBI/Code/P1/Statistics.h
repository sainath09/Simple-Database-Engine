#ifndef STATISTICS_
#define STATISTICS_

#include "ParseTree.h"

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>


using namespace std;
typedef struct attStats{
	int subSetNum;
	unsigned long long numTuples;
	unsigned long long numDistinct;
}attStats;
typedef struct subSet{
	int subSetNum;
	map<string,unsigned long long> AttMap;
	unsigned long long numTuples;
	vector<string> relations;
}subSet;

class Statistics
{
private:
	map<int, subSet> subSetsMap;
	map<string, int> allsubSetMap;
public:
	Statistics();
	Statistics(Statistics &copyMe);
	~Statistics();
	//Uses UO Maps
	void AddRel(char *relName, int numTuples);
	void AddAtt(char *relName, char *attName, unsigned long long numDistincts);
	void CopyRel(char *oldName, char *newName);
	//Read write Functions
	void Read(const char *fromWhere);
	void Write(const char *fromWhere);
	
	void  Apply(struct AndList *parseTree, char *relNames[], int numToJoin);
	double Estimate(struct AndList *parseTree, char **relNames, int numToJoin);

	attStats getattStats(string attr);
	void adjustSelectivityFactor(string attName, vector<string> &eachAtts, double& eachSelFactor, int code);
	pair<pair<unsigned long long, double>, vector<int> > calcEstimate(const struct AndList *andList);

};

#endif
