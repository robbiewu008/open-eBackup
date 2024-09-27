#!/bin/bash

read -s pass
pass=$(echo $pass|sed 's/\$/\\\$/g' | sed 's/\@/\\\@/g' | sed 's/\%/\\\%/g' | sed 's/\^/\^/g' | sed 's/\*/\*/g')
expect <<EOF
spawn gs_guc encrypt -M server -D ${GAUSSDATA}
expect {
    "Password:" { send "${pass}\r"; }
}
expect eof
EOF