#!/bin/bash

#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Function 容器的就绪检查
# revise note
########################################

function check_application_and_port() {
  curl -kv --connect-timeout 3 --max-time 3 127.0.0.1:elasticsearch_port 2>&1 | grep 'Connected'
  if [[ $? == 0 ]]; then
    echo "Detect connection to elasticsearch successfully"
    exit 0
  else
    echo "Detect connection to elasticsearch failed"
    exit 1
  fi
}

function main() {
  check_application_and_port
}

main
