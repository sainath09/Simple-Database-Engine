 
%{

	#include "ParseTree.h" 
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <iostream>

	extern "C" int yylex();
	extern "C" int yyparse();
	extern "C" void yyerror(char *s);
  
	
	// these data structures hold the result of the parsing
	struct CreateTable *createTable;
	struct FuncOperator *finalFunction; // the aggregate function (NULL if no agg)
	struct TableList *tables; // the list of tables and aliases in the query
	struct AndList *boolean; // the predicate in the WHERE clause
	struct NameList *groupingAtts; // grouping atts (NULL if no grouping)
	struct NameList *attsToSelect; // the set of attributes in the SELECT (NULL if no such atts)
	struct Insert *insertinto;
	int distinctAtts; // 1 if there is a DISTINCT in a non-aggregate query 
	int distinctFunc;  // 1 if there is a DISTINCT in an aggregate query
	char *droptablename;
	int outputmode;
	char *outputfile;
        bool DDL;
%}

// this stores all of the types returned by production rules
// this stores all of the types returned by production rules
%union {
	struct TableAtts *mytableatts;
 	struct FuncOperand *myOperand;
	struct FuncOperator *myOperator; 
	struct TableList *myTables;
	struct ComparisonOp *myComparison;
	struct Operand *myBoolOperand;
	struct OrList *myOrList;
	struct AndList *myAndList;
	struct NameList *myNames;
	struct CrAttr *crattr;
	struct CreateTable *mycreatetables;
	char *actualChars;
	char whichOne;
}

%token <actualChars> Name
%token <actualChars> Float
%token <actualChars> Int
%token <actualChars> String
%token SELECT
%token GROUP 
%token DISTINCT
%token BY
%token FROM
%token WHERE
%token SUM
%token AS
%token AND
%token OR
%token CREATE 
%token TABLE
%token HEAP
%token SORTED
%token INSERT
%token INTO
%token DROP
%token SET
%token OUTPUT
%token STDOUT
%token NONE
%token ON


%type <myOrList> OrList
%type <myAndList> AndList
%type <myOperand> SimpleExp
%type <myOperator> CompoundExp
%type <whichOne> Op 
%type <myComparison> BoolComp
%type <myComparison> Condition
%type <myTables> Tables
%type <myBoolOperand> Literal
%type <myNames> Atts
%type <mytableatts> Createattributes
%type <crattr> Newatt
%type <myNames>sortattributes


%start SQL


//******************************************************************************
// SECTION 3
//******************************************************************************
/* This is the PRODUCTION RULES section which defines how to "understand" the 
 * input language and what action to take for each "statment"
 */

%%

SQL: SELECT WhatIWant FROM Tables WHERE AndList
{
	tables = $4;
	boolean = $6;	
	groupingAtts = NULL;
	droptablename=NULL;
	createTable=NULL;
	groupingAtts=NULL;
	insertinto=NULL;
	outputmode=0;
        DDL=false;
}

| SELECT WhatIWant FROM Tables WHERE AndList GROUP BY Atts
{
	tables = $4;
	boolean = $6;	
	groupingAtts = $9;
	droptablename=NULL;
	createTable=NULL;
	insertinto=NULL;
	outputmode=0;
        DDL=false;
}
|
SELECT WhatIWant FROM Tables
{
	tables = $4;
	//boolean = $6;	
	groupingAtts = NULL;
	droptablename=NULL;
	createTable=NULL;
	groupingAtts=NULL;
	insertinto=NULL;
	outputmode=0;
        DDL=false;
}
| CREATE TABLE Name '(' Createattributes ')' AS HEAP
{
	createTable=(struct CreateTable *) malloc (sizeof (struct CreateTable));
	createTable->tableName=$3;
	createTable->sortkeys=NULL;
	createTable->atts=$5;
	droptablename=NULL;
	insertinto=NULL;
	outputmode=0;
        DDL=true;
}
| CREATE TABLE Name '(' Createattributes ')' AS SORTED ON sortattributes
{
	createTable=(struct CreateTable *) malloc (sizeof (struct CreateTable));
	createTable->tableName=$3;
	createTable->sortkeys=$10;
	createTable->atts=$5;
	droptablename=NULL;
	insertinto=NULL;
	outputmode=0;
        DDL=true;
}
| INSERT  Name  INTO Name
{
	insertinto=(struct Insert *)malloc(sizeof(struct Insert));
	insertinto->filename=$2;
	insertinto->dbfile=$4;
	droptablename=NULL;
	createTable=NULL;
	tables = NULL;
	boolean = NULL;	
	groupingAtts = NULL;
	createTable=NULL;
	groupingAtts=NULL;
	outputmode=0;
        DDL=true;

}
| DROP TABLE Name
{
	insertinto=NULL;
	createTable=NULL;
	tables = NULL;
	boolean = NULL;	
	groupingAtts = NULL;
	createTable=NULL;
	groupingAtts=NULL;
	droptablename=$3;
	outputmode=0;
        DDL=true;
}
| SET OUTPUT STDOUT
{
	outputmode=1;
        DDL=true;
}
| SET OUTPUT Name
{	
	outputmode=2;
	outputfile=$3;
        DDL=true;
}
| SET OUTPUT NONE
{
	outputmode=3;
        DDL=true;
};

sortattributes: Name ',' sortattributes
{
	$$=(struct NameList *) malloc (sizeof (struct NameList));
	$$->name=$1;
	$$->next=$3;
}
| Name
{
	$$=(struct NameList *) malloc (sizeof (struct NameList));
	$$->name=$1;;
	$$->next=NULL;
};

Createattributes: Newatt ',' Createattributes
{
	$$=(struct TableAtts *) malloc (sizeof (struct TableAtts));
	$$->Op=$1;
	$$->next=$3;
}
| Newatt
{
	$$=(struct TableAtts *) malloc (sizeof (struct TableAtts));
	$$->Op=$1;
	$$->next=NULL;
};

Newatt: Name Name
{
	$$=(struct CrAttr *) malloc (sizeof (struct CrAttr));
	$$->value=$1;
	$$->type=$2;
};

WhatIWant: Function ',' Atts 
{
	attsToSelect = $3;
	distinctAtts = 0;
}

| Function
{
	attsToSelect = NULL;
}

| Atts 
{
	distinctAtts = 0;
	finalFunction = NULL;
	attsToSelect = $1;
}

| DISTINCT Atts
{
	distinctAtts = 1;
	finalFunction = NULL;
	attsToSelect = $2;
	finalFunction = NULL;
};

Function: SUM '(' CompoundExp ')'
{
	distinctFunc = 0;
	finalFunction = $3;
}

| SUM DISTINCT '(' CompoundExp ')'
{
	distinctFunc = 1;
	finalFunction = $4;
};

Atts: Name
{
	$$ = (struct NameList *) malloc (sizeof (struct NameList));
	$$->name = $1;
	$$->next = NULL;
} 

| Atts ',' Name
{
	$$ = (struct NameList *) malloc (sizeof (struct NameList));
	$$->name = $3;
	$$->next = $1;
}

Tables: Name AS Name 
{
	$$ = (struct TableList *) malloc (sizeof (struct TableList));
	$$->tableName = $1;
	$$->aliasAs = $3;
	$$->next = NULL;
}

| Tables ',' Name AS Name
{
	$$ = (struct TableList *) malloc (sizeof (struct TableList));
	$$->tableName = $3;
	$$->aliasAs = $5;
	$$->next = $1;
}



CompoundExp: SimpleExp Op CompoundExp
{
	$$ = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));	
	$$->leftOperator = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));
	$$->leftOperator->leftOperator = NULL;
	$$->leftOperator->leftOperand = $1;
	$$->leftOperator->right = NULL;
	$$->leftOperand = NULL;
	$$->right = $3;
	$$->code = $2;	

}

| '(' CompoundExp ')' Op CompoundExp
{
	$$ = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));	
	$$->leftOperator = $2;
	$$->leftOperand = NULL;
	$$->right = $5;
	$$->code = $4;	

}

| '(' CompoundExp ')'
{
	$$ = $2;

}

| SimpleExp
{
	$$ = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));	
	$$->leftOperator = NULL;
	$$->leftOperand = $1;
	$$->right = NULL;	

}

| '-' CompoundExp
{
	$$ = (struct FuncOperator *) malloc (sizeof (struct FuncOperator));	
	$$->leftOperator = $2;
	$$->leftOperand = NULL;
	$$->right = NULL;	
	$$->code = '-';

}
;

Op: '-'
{
	$$ = '-';
}

| '+'
{
	$$ = '+';
}

| '*'
{
	$$ = '*';
}

| '/'
{
	$$ = '/';
}
;

AndList: '(' OrList ')' AND AndList
{
        // here we need to pre-pend the OrList to the AndList
        // first we allocate space for this node
        $$ = (struct AndList *) malloc (sizeof (struct AndList));

        // hang the OrList off of the left
        $$->left = $2;

        // hang the AndList off of the right
        $$->rightAnd = $5;

}

| '(' OrList ')'
{
        // just return the OrList!
        $$ = (struct AndList *) malloc (sizeof (struct AndList));
        $$->left = $2;
        $$->rightAnd = NULL;
}
;

OrList: Condition OR OrList
{
        // here we have to hang the condition off the left of the OrList
        $$ = (struct OrList *) malloc (sizeof (struct OrList));
        $$->left = $1;
        $$->rightOr = $3;
}

| Condition
{
        // nothing to hang off of the right
        $$ = (struct OrList *) malloc (sizeof (struct OrList));
        $$->left = $1;
        $$->rightOr = NULL;
}
;

Condition: Literal BoolComp Literal
{
        // in this case we have a simple literal/variable comparison
        $$ = $2;
        $$->left = $1;
        $$->right = $3;
}
;

BoolComp: '<'
{
        // construct and send up the comparison
        $$ = (struct ComparisonOp *) malloc (sizeof (struct ComparisonOp));
        $$->code = LESS_THAN;
}

| '>'
{
        // construct and send up the comparison
        $$ = (struct ComparisonOp *) malloc (sizeof (struct ComparisonOp));
        $$->code = GREATER_THAN;
}

| '='
{
        // construct and send up the comparison
        $$ = (struct ComparisonOp *) malloc (sizeof (struct ComparisonOp));
        $$->code = EQUALS;
}
;

Literal : String
{
        // construct and send up the operand containing the string
        $$ = (struct Operand *) malloc (sizeof (struct Operand));
        $$->code = STRING;
        $$->value = $1;
}

| Float
{
        // construct and send up the operand containing the FP number
        $$ = (struct Operand *) malloc (sizeof (struct Operand));
        $$->code = DOUBLE;
        $$->value = $1;
}

| Int
{
        // construct and send up the operand containing the integer
        $$ = (struct Operand *) malloc (sizeof (struct Operand));
        $$->code = INT;
        $$->value = $1;
}

| Name
{
        // construct and send up the operand containing the name
        $$ = (struct Operand *) malloc (sizeof (struct Operand));
        $$->code = NAME;
        $$->value = $1;
}
;


SimpleExp: 

Float
{
        // construct and send up the operand containing the FP number
        $$ = (struct FuncOperand *) malloc (sizeof (struct FuncOperand));
        $$->code = DOUBLE;
        $$->value = $1;
} 

| Int
{
        // construct and send up the operand containing the integer
        $$ = (struct FuncOperand *) malloc (sizeof (struct FuncOperand));
        $$->code = INT;
        $$->value = $1;
} 

| Name
{
        // construct and send up the operand containing the name
        $$ = (struct FuncOperand *) malloc (sizeof (struct FuncOperand));
        $$->code = NAME;
        $$->value = $1;
}
;

%%
