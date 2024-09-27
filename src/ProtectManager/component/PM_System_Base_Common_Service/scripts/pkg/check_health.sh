#!/bin/bash

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
