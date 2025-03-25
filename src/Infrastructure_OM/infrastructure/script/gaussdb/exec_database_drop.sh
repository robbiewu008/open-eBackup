#!/bin/bash
set -e

GAUSSDB_PORT=6432

export GAUSSHOME=/usr/local/gaussdb/app
export LD_LIBRARY_PATH=/usr/local/gaussdb/app/lib

database=$(/usr/local/gaussdb/app/bin/gsql postgres -p ${GAUSSDB_PORT} -At -c 'select datname, datdba, encoding from pg_database;' | grep "$1")

/usr/local/gaussdb/app/bin/gsql postgres -p ${GAUSSDB_PORT} -c "clean connection to all force for database $1;"
/usr/local/gaussdb/app/bin/gsql postgres -p ${GAUSSDB_PORT} -c "DROP DATABASE IF EXISTS $1;"

IFS='|' read -r db_name datdba encoding <<< "$database"

if [ "${encoding}" -eq 7 ]; then  # utf-8 encode
  cmd="CREATE DATABASE ${db_name} dbcompatibility = 'PG' ENCODING 'UTF8' TEMPLATE template0;"
else
  cmd="CREATE DATABASE ${db_name} dbcompatibility = 'PG';"
fi

/usr/local/gaussdb/app/bin/gsql postgres -p ${GAUSSDB_PORT} -c "${cmd}"

if [ "$datdba" -ne 10 ]; then
  /usr/local/gaussdb/app/bin/gsql postgres -p ${GAUSSDB_PORT} -c "ALTER DATABASE ${db_name} OWNER TO generaldb;"
fi
