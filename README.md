# Database Engine

This is a simple in-memory database engine to run  SQL queries.

We implemented a database engine that creates, Inserts and perform simple queries like sum and group By  for a dataset to understand the basics of any SQL query engine.
We implemented 2 diffeerent types of data structures for storing it.

  - [X] Heap based implementation 
	-  Each record of data sits on top of an other. There is no particular order in which data is sequenced.
-  [X] Sorted file Implementation
	- Data is place in the file after sorting on one or more columns defined in the query.
	- Sorting makes query operations on sorted columns efficient.
- [ ]  We planned to implement B+ trees as in any other database engine but were not successful. Any one can use this code and implement B+ trees.




### Implementation Details

We developed this engine incrementally.
*  We started it of with implementing the heap data base engine.
	* Record.cc
		* It contains all the functions required to process one record. For example toBinary and fromBinary functions 
		convert data to and from binary format to store them in .bin file.
		* composeRecord makes the record with given schema and the source file name.
	* File.cc 
		* It is the higher abstraction to Record in Db. File object contains number of records based on block size.
		* The default block size we used as in most of the database engines ins 4 Kb.
	* Schema.cc, Catalog.cc
		* They contain implemtation fuctions for schema of any table in the database. Schema of the database  is 				stored in catalog file.
	*  AbstractDbFile.cc, DBFile.cc, Heap.cc, Sort.cc
		* They implement heap data structure and sort data structure. DBFile objects abstracts the heap or sort data 				structure. 
	* BigQ.cc , Pipe.cc, ComparisionEngine.cc and Comparsion.cc
		* BigQ uses priorty queues to sort the records with order made by ordermaker.
		* It maintains two pipes inputPipe and outputPipe. Data is fed into the input pipe which is sorted based on ** External Merge Sort Technique ** 
		* BigQs are used not only in the sorted data structure but also to perform Join and Group By operations.
	

There are few different basic operations that our database can perform
Most of the operations in the database are paralized.
-  Join
-  Select from File
-  Select from a pipe - if my db is already in memory.
- Group By
- Sum over a column
- Project - squezes the record to required columns by discarding others.
- duplicate removal - removes duplicate records after sorting using BigQ.

####  Example Queries 
SELECT ps.ps_partkey ,<br />
	ps.ps_suppkey , <br />
	ps.ps_availqty , <br />
	ps.ps_supplycost , <br /> 
	ps.ps_comment  <br />
 	FROM partsupp AS ps WHERE (ps.ps_supplycost < 1.03) <br />
 
SELECT SUM (l.l_extendedprice * (1 - l.l_discount)), l.l_orderkey, o.o_orderdate, o.o_shippriority <br /> 
	    FROM customer AS c, orders AS o, lineitem AS l  <br />
	   WHERE (c.c_mktsegment = 'BUILDING') AND <br /> 
           (c.c_custkey = o.o_custkey) AND (l.l_orderkey = o.o_orderkey) AND <br /> 
	   (l.l_orderkey < 100 OR o.o_orderkey < 100) <br />
	   GROUP BY l_orderkey, o_orderdate, o_shippriority <br />

#### Key Words
Keywords | Keywords
------------- | ------------
SELECT | GROUP 
DISTINCT | BY
FROM | WHERE
SUM | AS
AND | OR
CREATE | TABLE
HEAP | SORTED
INSERT | INTO
DROP | SET
OUTPUT | STDOUT
NONE | ON
#### Data Types supported
- INTEGER
- STRING
- DOUBLE

  We tested our code using 1 Gb TPCH-C data set.
###To Start Querying 

Download the tpch data with the following commands.
```
cd settings/tpch-dbgen-master
```
```
make
```
to genetrate 10Mb data
```
./dbgen -s 0.01
```
to generate 1Gb data
```
./dbgen -s 1
```
```
mv *.tbl  ../../data
```

This project is tested with 1 Gb data. 

To run the queries - 
```
make main
```
```
./main
```
You will be promted to enter queries to execute.

There are some sample queires in 
  doc/create-insert-queries.txt and QueriesToExecute.


###Limitations
- All key words are case sensitive. You cannot go back and edit the query. This is the limitation from bison parser which we are working on fixing it.
-  There are few seg faults due to Join operation.
- Although me made sure all operations are done in parallel, Join operation take a lot more time than any other 
operation. So, We let the join thread complete before starting other processes.

If there are segmentaion faults - 
- Make sure your data and bin file paths are correct.
- Make sure catalog file is not deleted. 
- Do not delete bin files. Drop them through the queries.

