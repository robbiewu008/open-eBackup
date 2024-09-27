#!/bin/bash
GAUSSDB_PORT=6432

export GAUSSHOME=/usr/local/gaussdb/app;
export LD_LIBRARY_PATH=/usr/local/gaussdb/app/lib;/usr/local/gaussdb/app/bin/gsql postgres -p ${GAUSSDB_PORT} -c "DROP DATABASE IF EXISTS $1;"
