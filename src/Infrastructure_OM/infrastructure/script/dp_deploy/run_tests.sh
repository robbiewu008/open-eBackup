#!/bin/bash

BASEDIR=$(dirname $0)

CLIENT_PATH=${BASEDIR}/client
MIN_COVERAGE=50
python3 -m pip install pytest coverage pytest-cov
if [ $? -ne 0 ];then
    echo "install requirements failed"
    exit 1
fi
python3 -m pip install -r ${BASEDIR}/requirements.txt
if [ $? -ne 0 ];then
    echo "install requirements failed"
    exit 1
fi
pytest --cov=${CLIENT_PATH} --cov-report=xml --cov-report=html --cov-branch --junitxml=unit_report.xml
if [ $? -ne 0 ];then
    echo "exec pytest failed"
    exit 1
fi
python3 -m coverage report --fail-under=${MIN_COVERAGE}
if [ $? -ne 0 ];then
    echo "get coverage report failed"
    exit 1
fi
exit 0