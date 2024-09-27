#!/bin/bash

if [ $# -ne 0 ]; then
  echo "param is error"
  exit 1
fi

timezoneJson=$(curl http://127.0.0.1:5555/deviceManager/rest/dorado/system_timezone)

if [[ $a == *"CMO_SYS_TIME_ZONE_NAME"* ]]; then
  echo "${timezoneJson}"
fi
