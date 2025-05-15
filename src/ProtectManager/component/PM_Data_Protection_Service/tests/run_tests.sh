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

BASEDIR=$(dirname $0)

APP_PATH=${BASEDIR}/../src
COVERAGERC=.coveragerc
MIN_COVERAGE=10

module_name=$1

export PYTHONPATH=${APP_PATH}
python3 -m pip install pytest coverage pytest-cov
if [ $? -ne 0 ];then
    echo "install requirements failed"
    exit 1
fi
# 这里requirements 我们没有.txt尾缀 坑啊
python3 -m pip install --trusted-host  mirrors.tools.huawei.com -r ${APP_PATH}/requirements
if [ $? -ne 0 ];then
    echo "install requirements failed"
    exit 1
fi
if [[ -z "$module_name" ]];then
    pytest --cov-config ${COVERAGERC} --cov=${APP_PATH} --cov-report=xml --cov-report=html --cov-branch --junitxml=unit_report.xml
else
    pytest --cov-config ${COVERAGERC} --cov=${APP_PATH}/app/$module_name --cov-report=xml --cov-report=html --cov-branch --junitxml=unit_report.xml
fi
if [ $? -ne 0 ];then
    echo "exec pytest failed"
    exit 1
fi
python3 -m coverage report --fail-under=${MIN_COVERAGE}
if [ $? -ne 0 ];then
    echo "get coverage report failed"
    exit 1
fi
rm -rf /opt/logpath
exit 0