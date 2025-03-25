#!/bin/bash

#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Function 容器的健康检查
# revise note
########################################

function check_application_and_port() {
  netstat -tunple | grep elasticsearch_port >>/dev/null &&
    ps aux | grep elasticsearch | grep -v grep >>/dev/null
}

function main() {
  check_application_and_port
}

main
