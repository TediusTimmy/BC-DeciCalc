#!/bin/sh -x

rm -rf ../bin/cov
rm ../bin/*
rm ../obj/*.o
find .. -name '*.gcno' -exec rm \{\} \;
find .. -name '*.gcda' -exec rm \{\} \;
