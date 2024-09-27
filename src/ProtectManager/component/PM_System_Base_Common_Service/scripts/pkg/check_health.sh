#!/bin/bash

#########################################
# Copyright (c) 2021-2021 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function 容器的健康检查
# revise note
########################################

function check_health()
{
  sh /app/check_ready.sh
  if [ $? -eq 1 ]; then
    exit 1
  else
    exit 0
  fi
}

function main()
{
    check_health
}

main
