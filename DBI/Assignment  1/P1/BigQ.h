#ifndef BIGQ_H
#define BIGQ_H
#include <pthread.h>
#include <iostream>
#include "Pipe.h"
#include "File.h"
#include "Record.h"
#include <stdlib.h>
#include <vector>
#include <map>
#include <string>
#include <queue>
#include <math.h>

#define delVar(x) {delete x; x = NULL;}
using namespace std;

OrderMaker *orderMaker;

static bool sortOrder(Record* r1,Record* r2){
	ComparisonEngine comp;
	if(comp.Compare(r1,r2,orderMaker)<0) return true;
	else return false;
}

class sortMergeOrder{

public:
	sortMergeOrder(){}

	bool operator()(Record* r1,Record* r2) {
		ComparisonEngine comp;
		if(comp.Compare(r1,r2,orderMaker)<0) return false;
		else return true;
	}
};
class BigQ {


public:

	BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);
	~BigQ ();
};

#endif
