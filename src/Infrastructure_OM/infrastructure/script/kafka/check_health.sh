#!/bin/bash

#########################################
# Copyright (c) 2012-2015 Huawei .
# All rights reserved.
#
# Please send feedback to http://www.huawei.com
#
# Function ÈÝÆ÷µÄ½¡¿µ¼ì²é
# revise note
########################################

function check_application_and_port()
{
    netstat -tunple | grep kafka_port >> /dev/null &&
    ps aux | grep kafka | grep -v grep >> /dev/null
}

function main()
{
    check_application_and_port
}

main