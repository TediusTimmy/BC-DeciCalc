#!/bin/sh -x

cd ./src/Types
g++ -I../../include -I../../../BCNum -g -O0 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../Input
g++ -I../../include -I../../../BCNum -g -O0 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../Engine
g++ -I../../include -I../../../BCNum -g -O0 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../Parser
g++ -I../../include -I../../../BCNum -g -O0 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../../bin
ar -rcs Backwards.a ../obj/*.o
