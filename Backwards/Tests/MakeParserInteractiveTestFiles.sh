#!/bin/sh -x

./Clean.sh

cd ../src/Types
../../MakeDebug.sh
mv ./*.o ../../obj

cd ../Input
../../MakeDebug.sh
mv ./*.o ../../obj

cd ../Engine
../../MakeDebug.sh
mv ./*.o ../../obj

cd ../Parser
../../MakeDebug.sh
mv ./*.o ../../obj

cd ../../bin
g++ -o ParserTestFile -Wall -Wextra -Wpedantic -O0 -g -I../include -I../../BCNum ../Tests/ParserTestFromFile.cpp ../obj/*.o ../obj/*.a -lgmp
