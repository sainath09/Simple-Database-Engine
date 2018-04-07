!#/bin/bash
echo "Cleaning old compiled files"
make clean
echo "Compiling Test.out"
make test.out 
echo "OUTPUT FROM TEST 0"
./test.out 0
echo "OUTPUT FROM TEST 1"
./test.out 1
echo "OUTPUT FROM TEST 2"
./test.out 2
echo "OUTPUT FROM TEST 3"
./test.out 3
echo "OUTPUT FROM TEST 4"
./test.out 4
echo "OUTPUT FROM TEST 5"
./test.out 5
echo "OUTPUT FROM TEST 6"
./test.out 6
echo "OUTPUT FROM TEST 7"
./test.out 7
echo "OUTPUT FROM TEST 8"
./test.out 8
echo "OUTPUT FROM TEST 9"
./test.out 9
echo "OUTPUT FROM TEST 10"
./test.out 10
echo "OUTPUT FROM TEST 11"
./test.out 11

