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
g++ -o ParserTest -Wall -Wextra -Wpedantic -g --coverage -O0 -I../../../External/googletest/include -I../include -I../../BCNum -I../../Backwards/include ../Tests/ParserTest.cpp ../Tests/SpreadSheetTest.cpp ../obj/*.o ../../../External/googletest/lib/libgtest.a ../../../External/googletest/lib/libgtest_main.a ../obj/*.a -lgmp
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --initial --directory ../src/Parser --directory ../include/Forwards/Parser --output-file Parser_Base.info
./ParserTest.exe
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --directory ../src/Parser --directory ../include/Forwards/Parser --directory . --output-file Parser_Run.info
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --add-tracefile Parser_Base.info --add-tracefile Parser_Run.info --output-file Parser.info
mkdir cov
../../../External/lcov/bin/genhtml --rc lcov_branch_coverage=1 --branch-coverage --output-directory cov Parser.info
