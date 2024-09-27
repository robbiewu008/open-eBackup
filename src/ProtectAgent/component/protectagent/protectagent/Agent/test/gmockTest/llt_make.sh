#!/bin/sh

# compile open_src

# compile sqlite3
gcc -c sqlite3.c -o sqlite3.o
ar -r sqlite3.a sqlite3.o

# compile tinyxml2.cpp

# compile with cmake


