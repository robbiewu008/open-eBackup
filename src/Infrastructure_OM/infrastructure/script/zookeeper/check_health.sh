#!/bin/bash

#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function �����Ľ������
# revise note
########################################

function check_application_and_port() {
  netstat -tunple | grep zookeeper_port >>/dev/null &&
    ps aux | grep QuorumPeerMain | grep -v grep >>/dev/null
}

function main() {
  check_application_and_port
}

main
