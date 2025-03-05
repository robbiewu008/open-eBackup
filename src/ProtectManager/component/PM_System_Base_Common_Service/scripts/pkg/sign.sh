#!/bin/bash
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

set +x

#动作 生成私钥，对信息签名
ACTION=$1

PRIVATE_KEY_PATH=/app/agent/keyfile/privatekey/privateKey_encrypt.pem
PUBLIC_KEY_PATH=/app/agent/keyfile/publickey/upgrade_public_key.pem

# generate private key
if [ "$ACTION" = "generatePrivateKey" ]; then
  read -s -n 384 password
  printf -v pass '%q' "$password"
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
  printf -v pass '%q' "$password"
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
  printf -v pass '%q' "$password"
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
  printf -v pass '%q' "$password"
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