#!/bin/sh
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
pass=

CONFIG="/app/bin/template.openssl.cnf"
AGENT_CONFIG="/app/bin/agent.openssl.cnf"
ROOT_PASS=''
SUBJ="/C=CN/O=Huawei/CN=OceanProtect"
SYS_CONF="/app/cert/conf"

value=
DIR=`sed -n '/^dir/p' ${CONFIG} | awk '{print $3}'`

read_conf() {
  local t=`sed -n "/^$1/p" ${CONFIG} | awk '{print $3}'`
  value=${t/\$dir/$DIR}
}

read_conf "certs"
CERTS=$value
read_conf "crl_dir"
CRL_DIR=$value
read_conf "new_certs_dir"
NEW_CERTS_DIR=$value
read_conf "private_key"
ROOT_KEY=$value
read_conf "certificate"
ROOT_CRT=$value

init_dir() {
  mkdir -p $CERTS
  mkdir -p $CRL_DIR
  mkdir -p $NEW_CERTS_DIR
  mkdir -p $DIR/private
  touch $DIR/index.txt
  touch $DIR/index.txt.attr
  echo "01" > $DIR/serial
  mkdir -p $SYS_CONF
  cp /app/conf/dpa.properties $SYS_CONF
}

_initca() {
  read -s RootPassword
  if [[ ! -f "${ROOT_KEY}" ]]; then
    init_dir
    _genca ${RootPassword} $2
  fi
}

_init_agent_ca() {
  read -s RootPassword
  _gen_agent_ca ${RootPassword} $2
}

_genca() {
  ROOT_PASS="$1"
  CA_SUBJ="$2"
  #gen root key
  _key -passout "${ROOT_PASS}" -out "${ROOT_KEY}" 3072
  #gen root csr
  _req -config "${CONFIG}" -passin "${ROOT_PASS}" -key "${ROOT_KEY}" -out $DIR/root.csr -subj "${CA_SUBJ}"
  #gen root crt
  printf -v send_root_pass '%q' "${ROOT_PASS}"
expect <<EOF
  spawn openssl ca -config ${CONFIG} -selfsign -in $DIR/root.csr -out ${ROOT_CRT} -rand_serial -extensions v3_ca
  expect {
    "Enter pass phrase for" {send -- "${send_root_pass}\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "No certificate matches private key" {exit 1}
    "error" {exit 1}
  }
EOF
}

_gen_agent_ca() {
  AGENT_PRI_KEY_PATH="/app/cert/internal/ProtectAgent/ca/ca.pem"
  AGENT_CA_PATH="/app/cert/internal/ProtectAgent/ca/ca.crt.pem"
  AGENT_CA_CSR="/app/cert/internal/ProtectAgent/ca/ca.csr"
  ROOT_PASS="$1"
  CA_SUBJ="$2"
  #gen root key
  _key -passout "${ROOT_PASS}" -out "${AGENT_PRI_KEY_PATH}" 3072
  #gen root csr
  _req -config "${CONFIG}" -passin "${ROOT_PASS}" -key "${AGENT_PRI_KEY_PATH}" -out "${AGENT_CA_CSR}" -subj "${CA_SUBJ}"
  #gen root crt
  printf -v send_root_pass '%q' "${ROOT_PASS}"
expect <<EOF
    spawn openssl x509 -req -in "${AGENT_CA_CSR}" -sha256 -signkey "${AGENT_PRI_KEY_PATH}" -out "${AGENT_CA_PATH}" -days 18250 -extensions v3_ca -extfile "${CONFIG}"
  expect {
    "Enter pass phrase for" {send -- "${send_root_pass}\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "No certificate matches private key" {exit 1}
    "error" {exit 1}
  }
EOF
}

_decryptKey2Pkcs8WithNoPwd() {
  local inKey=
  for i in $(seq $#)
  do
    if [[ "$1" == "-in" ]]; then
      inKey="$2"
      shift 2
    fi
  done
  openssl pkcs8 -topk8 -in "${inKey}" -nocrypt
}

_decryptKey2Pkcs8() {
  local inKey=
  local PasswordIn=
  for i in $(seq $#)
  do
    if [[ "$1" == "-in" ]]; then
      inKey="$2"
      shift 2
    elif [[ "$1" == "-passin" ]]; then
      read -s -r InPasswordSensitiveParam
      PasswordIn=${InPasswordSensitiveParam}
      shift 2
    fi
  done
  echo "${PasswordIn}" | openssl pkcs8 -topk8 -in "${inKey}" -nocrypt -passin stdin
}

#read_pass cert_name
read_pass() {
  if [[ "$pass" != "" ]]; then
    return
  fi
  read -p "Please input the password for $1:" -s pass
  [[ ${#pass} -ge 8 ]] &&
    echo "${pass}" | grep -Eq '[A-Z]' &&
    echo "${pass}" | grep -Eq '[a-z]' &&
    echo "${pass}" | grep -Eq '[0-9]' &&
    echo "${pass}" | grep -Eq '[~!@#$%&*()^]' &&
    echo "" && return 0
  echo "
The password must meet the following requirements:
1. Contains at least 8 characters.
2. Contains uppercase letters.
3. Contains lowercase letters.
4. Contains digits.
5. Contains special characters. e.g.: ~!@#$%&*()^"
  exit 1
}

#gen_key cert_name out
gen_key() {
  local name=$1
  shift
  if [[ -f "$1" ]]; then
    return
  fi
  _key -t "$name cert" "$@"
}

verify() {
  openssl x509 -noout -text -in "$1"
  if [[ $? -ne 0 ]]; then
    return 1
  fi
  if [[ "$2" != "" ]] && [[ -f "$2" ]]; then
    openssl verify -CAfile "$2" "$1"
    if [[ $? -ne 0 ]]; then
      return 1
    fi
  fi
  return 0
}

issue() {
#  local cert=$1
  pass=$1
  read_pass
  local config=$2
  local req=$3
  local out=$4
  shift 5
  local folder=$(dirname "$out")
  mkdir -p "$folder"
#  echo "Create the ${cert} certificate"
 # 通过CA证书生成服务端证书文件
  printf -v send_pass '%q' "${pass}"
  local openssl_ssl_config=${config}
  result=$(echo $out | grep "ProtectAgent")
  if [[ "$result" != "" ]]
  then
    openssl_ssl_config=${AGENT_CONFIG}
  fi
expect <<EOF
  spawn openssl ca -batch -config ${openssl_ssl_config} -notext -md sha256 -in "$req" -out "$out" -rand_serial -extensions "$@"
  expect {
    "Enter pass phrase" {send -- "${send_pass}\n"}
  }
  expect eof
EOF
}

_gencert() {
  local rootPassFile=
  local certPassFile=
  local keyPath=
  local reqPath=
  local certPath=
  local subj=

  read -s rootPassParam
  read -s certPassParam

  for i in $(seq $#)
  do
    if [[ "$1" == "-passin" ]]; then
      rootPassFile=${rootPassParam}
      shift 2
    elif [[  "$1" == "-passout" ]]; then
      certPassFile=${certPassParam}
      shift 2
    elif [[  "$1" == "-key" ]]; then
      keyPath="$2"
      shift 2
    elif [[  "$1" == "-req" ]]; then
      reqPath="$2"
      shift 2
    elif [[  "$1" == "-out" ]]; then
      certPath="$2"
      shift 2
    elif [[  "$1" == "-subj" ]]; then
      subj="$2"
      shift 2
    fi
  done

  _key -passout "$certPassFile" -out "$keyPath" 3072
  _req -passin "$certPassFile" -key "$keyPath" -out "$reqPath" -subj "$subj"
  _iss -pass "$rootPassFile" -req "$reqPath" -out "$certPath" -extensions device_cert "$@"
}

#help:_iss <-config config> <-req req> <-out out> <-pass passwd>
_iss() {
  local req=
  local out=
  local config="${CONFIG}"
  for i in $(seq $#)
  do
    if [[ "$1" == "-config" ]]; then
      config="$2"
      shift 2
    elif [[  "$1" == "-pass" ]]; then
      pass="$2"
      shift 2
    elif [[  "$1" == "-req" ]]; then
      req="$2"
      shift 2
    elif [[  "$1" == "-out" ]]; then
      out="$2"
      shift 2
    fi
  done
  issue "$pass" "$config" "$req" "$out" "$@"
}

#help:_decrypt <out> [option...]
_decrypt() {
  local in=
  local out=
  local pwd=
  for i in $(seq $#)
  do
    if [[ "$1" == "-in" ]]; then
      in="$2"
      shift 2
    elif [[  "$1" == "-out" ]]; then
      out="$2"
      shift 2
    elif [[  "$1" == "-passin" ]]; then
      read -s -r PasswordSensitiveParam
      pwd=${PasswordSensitiveParam}
      shift 2
    fi
  done
  echo "${pwd}" | openssl rsa -in "${in}" -text -passin stdin
}

_reEncryptKey() {
  local inCertPath=
  local outCertPath=
  local inPassword=
  local outPassword=
  for i in $(seq $#)
  do
    if [[ "$1" == "-in" ]]; then
      inCertPath="$2"
      shift 2
    elif [[ "$1" == "-out" ]]; then
      outCertPath="$2"
      shift 2
    elif [[ "$1" == "-passin" ]]; then
      read -s -r InPasswordSensitiveParam
      inPassword=${InPasswordSensitiveParam}
      shift 2
    elif [[ "$1" == "-passout" ]]; then
      read -s -r OutPasswordSensitiveParam
      outPassword=${OutPasswordSensitiveParam}
      shift 2
    fi
  done
  inPassword="${inPassword//\\/\\\\}"
  printf -v inPassword '%q' "${inPassword}"
  outPassword="${outPassword//\\/\\\\}"
  printf -v outPassword '%q' "${outPassword}"
expect <<EOF
  spawn openssl rsa -in "$inCertPath" -aes256 -out "$outCertPath"
  expect {
  "Enter pass phrase" {send -- "${inPassword}\n";exp_continue;}
  "Enter PEM pass phrase" {send -- "${outPassword}\n";exp_continue;}
  "Verifying" {send -- "${outPassword}\n";}
  "No certificate matches private key" {exit 1}
  "error" {exit 1}
  }
EOF
}

#help:_merge <out> [option...]
_merge() {
  local cert=
  local key=
  local pwd=
  local out=
  local exp=
  for i in $(seq $#)
  do
    if [[ "$1" == "-in" ]]; then
      cert="$2"
    elif [[ "$1" == "-inkey" ]]; then
      key="$2"
    elif [[ "$1" == "-passin" ]]; then
      read -s -r InPasswordSensitiveParam
      pwd=${InPasswordSensitiveParam}
    elif [[ "$1" == "-out" ]]; then
      out="$2"
    elif [[ "$1" == "-passout" ]]; then
      read -s -r OutPasswordSensitiveParam
      exp=${OutPasswordSensitiveParam}
    else
      break
    fi
    shift 2
  done
  pwd="${pwd//\\/\\\\}"
  printf -v pwd '%q' "${pwd}"
  exp="${exp//\\/\\\\}"
  printf -v exp '%q' "${exp}"
expect <<EOF
  spawn openssl pkcs12 -export -clcerts -in "${cert}" -inkey "${key}" -out "${out}"
  expect {
  "Enter pass phrase" {send -- "${pwd}\n";exp_continue;}
  "Enter Export" {send -- "${exp}\n";exp_continue;}
  "Verifying" {send -- "${exp}\n";}
  "No certificate matches private key" {exit 1}
  "error" {exit 1}
  }
EOF
}

#req_cert cert_name cn config key out ...
req_cert() {
  local cert=$1
  read_pass "$cert"
  local cn=$2
  if [[ "$cn" == "" ]]; then
    echo "Not provide common name for $cert cert"
    exit 1
  fi
  local config=$3
  local key=$4
  local out=$5
  shift 5
  _req -config "${config}" -cn "${cn}" -key "${key}" -out "${out}" "$@"
}

#help:_key <out> [option...]
_key() {
  local type="key"
  local path=
  local isRedis=
  for index in $(seq $#)
  do
    if [[ "$1" == "-t" ]]; then
      type="$2"
      shift 2
    elif [[ "$1" == "-passout" ]]; then
      pass="$2"
      shift 2
      # 如果读取的参数是SensitiveParam字符，则说明这是与另一个进程在交付，此时这个敏感参数必须从输出流里读取
      if [[ $pass == "SensitiveParam" ]]; then
        read -s PasswordSensitiveParam
        pass=${PasswordSensitiveParam}
      fi
    elif [[ "$1" == "redis" ]]; then
      pass="$2"
      isRedis="true"
      shift 2
      # 如果读取的参数是SensitiveParam字符，则说明这是与另一个进程在交付，此时这个敏感参数必须从输出流里读取
      if [[ $pass == "SensitiveParam" ]]; then
        read -s PasswordSensitiveParam
        pass=${PasswordSensitiveParam}
      fi
    elif [[ "$1" == "-out" ]]; then
      path="$2"
      shift 2
    else
      break;
    fi
  done
  if [[ "$path" == "" ]]; then
    echo "Not provide out path of key file"
    exit 1
  fi
  local folder=$(dirname "$path")
  read_pass "$type"
  mkdir -p "$folder"
  echo "Generate the key file"
  if [[ "$isRedis" == "true" ]]; then
    echo "generating key without password protection"
    openssl genrsa -f4 -out "${path}" "$@"
  else
    if [[ "$pass" == "" ]]; then
      echo "Not provide password param in function _key"
      exit 1
    fi
    echo "generating key with password protection"
    echo "$pass" | openssl genrsa -aes256 -f4 -passout stdin -out "${path}" "$@"
  fi
}

#help:_req <-config config> <-subj subj> <-cn cn> <-key key> <-out out>
_req() {
  local key=
  local out=
  local config=$CONFIG
  for i in $(seq $#)
  do
    if [[ "$1" == "-passin" ]]; then
      pass="$2"
      # 如果读取的参数是SensitiveParam字符，则说明这是与另一个进程在交付，此时这个敏感参数必须从输出流里读取
      if [[ $pass == "SensitiveParam" ]]; then
        read -s PasswordSensitiveParam
        pass=${PasswordSensitiveParam}
        if [[ "$pass" == "" ]]; then
          echo "Not provide password param in passin key"
          exit 1
        fi
      fi
    elif [[ "$1" == "redis" ]]; then
      isRedis="true"
      pass="$2"
      # 如果读取的参数是SensitiveParam字符，则说明这是与另一个进程在交付，此时这个敏感参数必须从输出流里读取
      if [[ $pass == "SensitiveParam" ]]; then
        read -s PasswordSensitiveParam
        pass=${PasswordSensitiveParam}
        if [[ "$pass" == "" ]]; then
          echo "Not provide password param in redis key"
          exit 1
        fi
      fi
    elif [[ "$1" == "-key" ]]; then
      key="$2"
      if [[ ! -f "$key" ]]; then
        echo "key file is not exist"
        exit 1
      fi
    elif [[ "$1" == "-out" ]]; then
      out="$2"
    elif [[ "$1" == "-config" ]]; then
      config="$2"
    else
      break
    fi
    shift 2
  done
  read_pass "$cert"
  local folder=$(dirname "$out")
  mkdir -p "$folder"
  if [[ "$isRedis" == "true" ]]; then
    echo "generating redis csr without keyPass"
    openssl req -config "${config}"  -key "${key}" -new -out "${out}" "$@"
  else
    echo "generating normal csr with keyPath"
    echo "$pass" | openssl req -config "${config}" -passin stdin -key "${key}" -new -sha256 -out "${out}" "$@"
  fi
}

#help:_root_cert <common-name>
_root_cert() { #test ! -f 1.root/certs/root.cert.pem
  local key=1.root/private/root.key.pem
  gen_key root "$key" 4096
  local cn=$1
  local config=$(dir=1.root name=root policy=policy_strict config)
  echo "100212" >1.root/serial
  req_cert root "$cn" "$config" "$key" "1.root/certs/root.cert.pem" -x509 -days 18250 -extensions v3_ca
}

#help:_intermediate_cert <common-name>
_intermediate_cert() { #test ! -f 2.intermediate/certs/intermediate.cert.pem
  local cn=$1
  call _root_cert "$cn"
  local key=2.intermediate/private/intermediate.key.pem
  gen_key intermediate "$key" 4096
  local config=$(dir=1.root name=root policy=policy_strict config)
  echo "100212" >2.intermediate/serial
  req_cert intermediate "$cn" "$config" "$key" "2.intermediate/csr/intermediate.csr.pem"
  issue intermediate "$config" "2.intermediate/csr/intermediate.csr.pem" "2.intermediate/certs/intermediate.cert.pem" -extensions v3_intermediate_ca
}

#help:_server_cert <common-name>
_server_cert() { #test ! -f 3.server/certs/server.cert.pem
  local cn=$1
  call _intermediate_cert "$cn"
  local key=3.server/private/server.key.pem
  gen_key server "$key" 4096
  local config=$(dir=2.intermediate name=intermediate policy=policy_loose config)
  req_cert server "$cn" "$config" "$key" "3.server/csr/server.csr.pem"
  issue server "$config" "3.server/csr/server.csr.pem" "3.server/certs/server.cert.pem" -extensions server_cert
}

#help:_client_cert <common-name>
_client_cert() { #test ! -f 4.client/certs/client.certs.pem
  local cn=$1
  call _intermediate_cert "$cn"
  local key=4.client/private/client.key.pem
  gen_key client "$key" 4096
  local config=$(dir=2.intermediate name=intermediate policy=policy_loose config)
  req_cert client "$cn" "$config" "$key" "4.client/csr/client.csr.pem"
  issue client "$config" "4.client/csr/client.csr.pem" "4.client/certs/client.certs.pem" -extensions client_cert
}

#help:_clean <all>|_clean [index...]|_clean
_clean() {
  if [[ "$1" == "all" ]]; then
    rm -rf 1.root
    rm -rf 2.intermediate
    rm -rf 3.server
    rm -rf 4.client
  elif [[ $# -gt 0 ]]; then
    for i in $(seq $#); do
      clean "$1"
      shift
    done
  else
    items="Certificate List:"
    for item in 1.root 2.intermediate 3.server 4.client; do
      if [[ -e "$item" ]]; then
        items="$items
        [+]$item"
      else
        items="$items
        [-]$item"
      fi
    done
    echo "$items"
    read -p "Please input the number of certificate to delete:" n
    for i in $n; do
      clean "$i"
    done
  fi
}

clean() {
  if [[ "$1" == "1" ]]; then
    item=1.root
  elif [[ "$1" == "2" ]]; then
    item=2.intermediate
  elif [[ "$1" == "3" ]]; then
    item=3.server
  elif [[ "$1" == "4" ]]; then
    item=4.client
  else
    return 1
  fi
  if [[ -e "$item" ]]; then
    rm -rf "$item"
  fi
}

#help:_help
_help() {
  echo "Usage:"
  sed -n '/^#help:/p' "$0" | sed 's/^#help://' | sed 's/|/\n/g' | sed 's/^_//' | sed 's/^ */    /g'
}

config() {
  local config=$(mktemp openssl.XXXXXXXX.cnf)
  local template=$(dirname "$0")/template.openssl.cnf
  sed -e "s#{{dir}}#${dir}#g" -e "s#{{name}}#${name}#g" -e "s#{{policy}}#${policy}#g" "$template" >"$config"
  echo "$config"
}

call() {
  # 由于使用到了eval函数，必须在此方法前，做出黑名单检验，不能有*?[]{}特殊符号
  WHITE_ARR=("/C=CN/O=Huawei/CN=*.dpa.svc.cluster.local")
  for i in $*
  do
    if [[ $i == *[*?\[\]\{\}+]* ]]
    then
      is_safe=1
      for white_str in ${WHITE_ARR[*]}
      do
        if [ $white_str = $i ]
        then
          is_safe=0
          break
        fi
      done
      if [ $is_safe -eq 1 ]
      then
        return 1
      fi
    fi
  done
  local method=$1
  local params=$(sed -n -e "/^$method()/p" "$0" | sed -e 's/^[^#]*//' | sed 's/^# *//')
  if [[ "$params" != "" ]]; then
    eval "$params"
    if [[ $? -ne 0 ]]; then
      return 1
    fi
  fi
  "$@"
}

main() {
  if [[ $# -eq 0 ]]; then
    _help
    return 1
  fi
  local method="_$1"
  shift
  local xtype=$(type -t "$method")
  if [[ "$xtype" != "function" ]]; then
    _help
    return 1
  fi
  call $method "$@"
}
main "$@"
code=$?
rm -f openssl.*.cnf 2>/dev/null
exit $code
