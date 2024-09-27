#!/bin/bash
function sync() {
    echo "start ntpdate gaussdb"
    /usr/sbin/ntpdate gaussdb
    hwclock -w
    echo "end ntpdate gaussdb"
}

function start() {
    max_period=3.0
    time_period=$(ntpdate -q gaussdb | grep 'ntpdate' | awk -F 'offset' '{print $2}')
    period_num=$(echo "$time_period" | grep -oP '\d*\.\d+')
    compare_result=$(echo "$period_num $max_period" | awk '{if ($1 > $2) {print 1} else {print 0}}')
    if [[ $compare_result -eq 1 ]]; then
      sync "$time_period"
    fi
}

function stop() {
     kill -9 `ps -ef | grep "ntpd" | grep -v "grep" | awk -F ' ' '{print $2}'`
}

if [ "$1" = "start" ]; then
    start
fi
if [ "$1" = "stop" ]; then
    stop
fi