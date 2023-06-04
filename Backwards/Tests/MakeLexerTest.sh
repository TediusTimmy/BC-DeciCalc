#!/bin/sh -x

./Clean.sh
cd ../src/Input
../../MakeTest.sh
mv ./*.o ../../obj
cd ../../bin
g++ -o LexerTest -Wall -Wextra -Wpedantic --coverage -O0 -I../../../External/googletest/include -I../include ../Tests/LexerTest.cpp ../obj/*.o ../../../External/googletest/lib/libgtest.a ../../../External/googletest/lib/libgtest_main.a
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --initial --directory ../src/Input --directory ../include/Backwards/Input --output-file Lexer_Base.info
./LexerTest.exe
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --directory ../src/Input --directory ../include/Backwards/Input --directory . --output-file Lexer_Run.info
../../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --add-tracefile Lexer_Base.info --add-tracefile Lexer_Run.info --output-file Lexer.info
mkdir cov
../../../External/lcov/bin/genhtml --rc lcov_branch_coverage=1 --branch-coverage --output-directory cov Lexer.info
