#!/bin/sh

g++ -I../../include -I../../../Backwards/include -I../../../BCNum -g --coverage -O0 -c -Wall -Wextra -Wpedantic *.cpp
