#!/bin/sh -x

cd ./src/Types
g++ -I../../include -I../../../Backwards/include -I../../../BCNum -O3 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../Input
g++ -I../../include -I../../../Backwards/include -I../../../BCNum -O3 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../Engine
g++ -I../../include -I../../../Backwards/include -I../../../BCNum -O3 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../Parser
g++ -I../../include -I../../../Backwards/include -I../../../BCNum -O3 -c -Wall -Wextra -Wpedantic *.cpp
mv ./*.o ../../obj

cd ../../bin
ar -rcs Forwards.a ../obj/*.o
