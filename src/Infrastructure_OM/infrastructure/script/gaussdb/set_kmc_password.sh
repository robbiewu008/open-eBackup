#!/bin/bash

function g_security_expect_param_escape() {
  [[ $# -ne 1 ]] && return 1
  local param="$1"
  local replace_pattern=("\\" "\"" "'" "$" "\`" ";" "|" "&" "[" "]" "<" ">" "-" "."
    "!" "~" "#" "^" "(" ")" "=" "{" "}" "/" " " $'\t' $'\v' $'\f' $'\r' $'\n' $'\b')
  [[ -z "${param}" ]] && return 1
  for ((i = 0; i < ${#replace_pattern[@]}; i++)); do
    param=${param//"${replace_pattern[i]}"/\\"${replace_pattern[i]}"}
  done
  echo "$param"
}

read -s pass
pass=$(g_security_expect_param_escape "${pass}")
expect <<EOF
spawn gs_guc encrypt -M server -D ${GAUSSDATA}
expect {
    "Password:" { send "${pass}\r"; }
}
expect eof
EOF
