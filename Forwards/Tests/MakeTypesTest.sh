#!/bin/sh -x

./Clean.sh
cd ../src/Types
../../MakeTest.sh
mv ./*.o ../../obj
cd ../../bin
g++ -o TypesTest -Wall -Wextra -Wpedantic --coverage -O0 -I../../../External/googletest/include -I../include -I../../BCNum -I../../Backwards/include ../Tests/TypesTest.cpp ../obj/*.o ../../../External/googletest/lib/libgtest.a ../../../External/googletest/lib/libgtest_main.a ../obj/*.a -lgmp
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --initial --directory ../src/Types --directory ../include/Forwards/Types --output-file Types_Base.info
./TypesTest.exe
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --directory ../src/Types --directory ../include/Forwards/Types --directory . --output-file Types_Run.info
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --add-tracefile Types_Base.info --add-tracefile Types_Run.info --output-file Types.info
mkdir cov
../../../External/lcov/bin/genhtml --rc lcov_branch_coverage=1 --branch-coverage --output-directory cov Types.info
