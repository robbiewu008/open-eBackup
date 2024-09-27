#!/bin/bash

set +x

#动作 生成私钥，对信息签名
ACTION=$1

PRIVATE_KEY_PATH=/app/agent/keyfile/privatekey/privateKey_encrypt.pem
PUBLIC_KEY_PATH=/app/agent/keyfile/publickey/upgrade_public_key.pem

# 特殊字符转义，防止send命令注入
transfor_special_characters() {
  local input_params=$1
  out_params=$input_params
  for((i=0;i<${#COMMON_PARAMS[@]};i++))
  do
      out_params=${out_params//${COMMON_PARAMS[i]}/\\${COMMON_PARAMS[i]}}
  done
  out_params=${out_params//\?/\\?}
  echo "${out_params}"
}

# generate private key
if [ "$ACTION" = "generatePrivateKey" ]; then
  read -s -n 384 password
  pass=$(transfor_special_characters "${password}")
expect <<EOF
  spawn openssl genrsa -aes256 -out ${PRIVATE_KEY_PATH} 3072
  expect {
    "Enter pass phrase for" {send -- "${pass}\n";exp_continue;}
    "Verifying - Enter pass phrase for" {send -- "${pass}\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "No certificate matches private key" {exit 1}
    "error" {exit 1}
  }
EOF
  exit 0
fi

# generate public key
if [ "$ACTION" = "generatePublicKey" ]; then
  read -s -n 384 password;
  pass=$(transfor_special_characters "${password}")
expect <<EOF
  spawn openssl rsa -pubout -in ${PRIVATE_KEY_PATH} -out ${PUBLIC_KEY_PATH}
  expect {
    "Enter pass phrase for" {send -- "${pass}\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "No certificate matches private key" {exit 1}
    "error" {exit 1}
  }
EOF
  exit 0
fi

# sign file
if [ "$ACTION" = "sign" ]; then
  read -rs PARAM_1
  read -rs PARAM_2
  read -s -n 384 password;
  pass=$(transfor_special_characters "${password}")
  SIGN_FILE=${PARAM_1}
  SIGN_RESULT_FILE=${PARAM_2}
expect <<EOF
  spawn openssl dgst -sha256 -out ${SIGN_RESULT_FILE} -sign ${PRIVATE_KEY_PATH} ${SIGN_FILE}
  expect {
    "Enter pass phrase for" {send -- "${pass}\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "No certificate matches private key" {exit 1}
    "error" {exit 1}
  }
EOF
  exit 0
fi

if [ "$ACTION" = "signOld" ]; then
  read -rs PARAM_1
  read -rs PARAM_2
  read -rs PARAM_3
  read -s -n 384 password
  SIGN_FILE=${PARAM_1}
  SIGN_RESULT_FILE=${PARAM_2}
  PRIVATE_KEY_PKCS8_PATH=${PARAM_3}
  pass=$(transfor_special_characters "${password}")
expect <<EOF
  spawn openssl dgst -sign ${PRIVATE_KEY_PKCS8_PATH} -out ${SIGN_RESULT_FILE} ${SIGN_FILE}
  expect {
    "Enter pass phrase for" {send -- "${pass}\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "y/n" {send -- "y\n";exp_continue;}
    "No certificate matches private key" {exit 1}
    "error" {exit 1}
  }
EOF
  exit 0
fi

# verify file
if [ "$ACTION" = "verify" ]; then
  read -rs PARAM_1
  read -rs PARAM_2
  read -rs PARAM_3
  VERIFY_RESULT_FILE=${PARAM_1}
  INPUT_FILE=${PARAM_2}
  SIGNATURE_FILE=${PARAM_3}
  openssl dgst -sha256 -out ${VERIFY_RESULT_FILE} -verify ${PUBLIC_KEY_PATH} -signature ${SIGNATURE_FILE} ${INPUT_FILE}
  exit 0
fi