#!/bin/sh -x

./Clean.sh

cd ../src/Types
../../MakeTest.sh
mv ./*.o ../../obj

cd ../Engine
../../MakeTest.sh
mv ./*.o ../../obj

cd ../../bin
g++ -o EngineTest -Wall -Wextra -Wpedantic --coverage -O0 -I../../../External/googletest/include -I../include -I../../BCNum ../Tests/ExpressionTest.cpp ../Tests/StdLibTest.cpp ../Tests/StatementTest.cpp ../obj/*.o ../../../External/googletest/lib/libgtest.a ../../../External/googletest/lib/libgtest_main.a ../obj/*.a -lgmp
#g++ -o EngineTest -Wall -Wextra -Wpedantic --coverage -O0 -I../External/googletest/include -I../include ../Tests/StatementTest.cpp ../obj/*.o ../External/googletest/lib/libgtest.a ../External/googletest/lib/libgtest_main.a ../obj/*.a
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --initial --directory ../src/Engine --directory ../include/Backwards/Engine --output-file Engine_Base.info
./EngineTest.exe
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --directory ../src/Engine --directory ../include/Backwards/Engine --directory . --output-file Engine_Run.info
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --add-tracefile Engine_Base.info --add-tracefile Engine_Run.info --output-file Engine.info
mkdir cov
../../../External/lcov/bin/genhtml --rc lcov_branch_coverage=1 --branch-coverage --output-directory cov Engine.info
