#ifndef STATISTICS_H
#define STATISTICS_H
#include "ParseTree.h"
#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

typedef struct {
	unordered_map <string, int> mapAttr;
	int numTuples;
	int numRel;
}structRel;

class Statistics
{
private:
	unordered_map <string, structRel> mapRel;
	double resFromEstimate;

public:
	Statistics();
	Statistics(Statistics &copyMe);	 // Performs deep copy
	~Statistics();


	void AddRel(char *relName, int numTuples);
	void AddAtt(char *relName, char *attName, int numDistincts);
	void CopyRel(char *oldName, char *newName);
	
	void Read(char *fromWhere);
	void Write(char *fromWhere);

	void  Apply(struct AndList *parseTree, char *relNames[], int numToJoin);
	double Estimate(struct AndList *parseTree, char **relNames, int numToJoin);

};

#endif
