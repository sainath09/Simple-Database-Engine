# Database Engine

This is a simple in-memory database engine to run simple SQl queries.
To run the code - 

download the tpch data with the following commands.
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
mv *.tbl 
```
```
../../data
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
-There are some sample queires in 
  doc/create-inser-queries.txt and QueriesToExecute.

If there are segmentaion faults - 
- Make sure your data and bin file paths are correct.
- Make sure catalog file is not deleted. 
- Do not delete bin files. Drop them through the queries.

