#!/bin/bash -x

rm -f Fixed.o
rm -f Integer.o

rm -f Fixed.gcda
rm -f Fixed.gcno
rm -f Integer.gcda
rm -f Integer.gcno

rm -f Base.info
rm -f Run.info
rm -f Full.info

rm -f Test-Test.gcda
rm -f Test-Test.gcno

rm -f Test.exe

rm -rf cov

if [ "$1" == "clean" ]; then
   exit
fi

g++ -c -Wall -Wextra -Wpedantic -g --coverage -O0 Fixed.cpp Integer.cpp
g++ -o Test -Wall -Wextra -Wpedantic -g --coverage -O0 -I../../External/googletest/include Test.cpp Fixed.o Integer.o ../../External/googletest/lib/libgtest.a ../../External/googletest/lib/libgtest_main.a -lgmp


if [ "$1" == "nocov" ]; then
   exit
fi

../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --initial --directory . --output-file Base.info
./Test.exe
../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --directory . --output-file Run.info
../../External/lcov/bin/lcov --rc lcov_branch_coverage=1 --add-tracefile Base.info --add-tracefile Run.info --output-file Full.info
mkdir cov
../../External/lcov/bin/genhtml --rc lcov_branch_coverage=1 --branch-coverage --output-directory cov Full.info
