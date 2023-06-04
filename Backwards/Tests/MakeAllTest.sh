#!/bin/sh -x

./Clean.sh

cd ../src/Types
../../MakeTest.sh
mv ./*.o ../../obj

cd ../Input
../../MakeTest.sh
mv ./*.o ../../obj

cd ../Engine
../../MakeTest.sh
mv ./*.o ../../obj

cd ../Parser
../../MakeTest.sh
mv ./*.o ../../obj

cd ../../bin
g++ -o AllTest -g -Wall -Wextra -Wpedantic --coverage -O0 -I../../../External/googletest/include -I../include -I../../BCNum ../Tests/DebuggerTest.cpp ../Tests/ExecutionTest.cpp ../Tests/ExpressionTest.cpp ../Tests/LexerTest.cpp ../Tests/ParserTest.cpp ../Tests/StatementTest.cpp ../Tests/StdLibTest.cpp ../Tests/TypesTest.cpp ../obj/*.o ../../../External/googletest/lib/libgtest.a ../../../External/googletest/lib/libgtest_main.a ../obj/*.a -lgmp
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --initial --directory ../src --directory ../include --output-file All_Base.info
./AllTest.exe
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --directory ../src --directory ../include --directory . --output-file All_Run.info
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --add-tracefile All_Base.info --add-tracefile All_Run.info --output-file All.info
mkdir cov
../../../External/lcov/bin/genhtml --rc lcov_branch_coverage=1 --branch-coverage --output-directory cov All.info
