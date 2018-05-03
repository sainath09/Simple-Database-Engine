#ifndef BIGQ_H
#define BIGQ_H
#include <iostream>
#include "Pipe.h"
#include "File.h"
#include "Record.h"
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <queue>
#include <math.h>
// We use this function quite extensively, may move it if time allows.
#define delVar(x) {delete x; x = NULL;}
using namespace std;

typedef struct {
	
	OrderMaker* order;
	int runlen;
	Pipe *iPipe;
	Pipe *oPipe;
} structBigQ;


class BigQ {


public:

	BigQ (Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);
	~BigQ ();
};

#endif
