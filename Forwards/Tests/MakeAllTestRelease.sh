#!/bin/sh -x

./Clean.sh

cd ../src/Types
g++ -I../../include -I../../../BCNum -I../../../Backwards/include -s -O3 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../Input
g++ -I../../include -I../../../BCNum -I../../../Backwards/include -s -O3 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../Engine
g++ -I../../include -I../../../BCNum -I../../../Backwards/include -s -O3 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../Parser
g++ -I../../include -I../../../BCNum -I../../../Backwards/include -s -O3 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../../bin
g++ -o AllTest -s -Wall -Wextra -Wpedantic -O3 -I../../../External/googletest/include -I../include -I../../BCNum -I../../Backwards/include ../Tests/ExpressionTest.cpp ../Tests/LexerTest.cpp ../Tests/ParserTest.cpp ../Tests/SpreadSheetTest.cpp ../Tests/TypesTest.cpp ../obj/*.o ../../../External/googletest/lib/libgtest.a ../../../External/googletest/lib/libgtest_main.a ../obj/*.a -lgmp
./AllTest.exe
