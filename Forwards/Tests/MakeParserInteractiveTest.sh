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
g++ -o ParserTestConsole -Wall -Wextra -Wpedantic -O0 -g -I../include -I../../BCNum -I../../Backwards/include ../Tests/ParserTestFromInput.cpp ../obj/*.o ../obj/*.a -lgmp
