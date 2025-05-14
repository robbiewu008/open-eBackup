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

function main(){
    local path=$1
    cd $path
    # yum install -y python3-devel
    echo "param2=$2"
    if [ $2 = "dev" ]; then
         pip3 install --no-index --find-links=$path ptvsd-5.0.0a12-py2.py3-none-any.whl
        cd $path
    fi
    pip3 install --no-index --find-links=$path confluent-kafka==1.8.2
    pip3 install --no-index --find-links=$path APScheduler==3.10.0
    # 替换pg_config配置
    cd $path
    pip3 install --no-index --find-links=$path requests==2.32.0
    pip3 install --no-index --find-links=$path urllib3==1.26.19
    pip3 install --no-index --find-links=$path fastapi==0.110.0
    pip3 install --no-index --find-links=$path pydantic==1.10.5
    pip3 install --no-index --find-links=$path PyJWT==2.4.0
    pip3 install --no-index --find-links=$path python-json-logger==2.0.2
    pip3 install --no-index --find-links=$path pytz==2023.3
    pip3 install --no-index --find-links=$path redis==4.5.4
    pip3 install --no-index --find-links=$path tzdata==2024.2

    # 安装SQLAlchemy，修改/SQLAlchemy-1.4.49/lib/sqlalchemy/dialects/postgresql/base.py版本匹配地方
    tar xzf SQLAlchemy-1.4.49.tar.gz
    sed -i 's/v,/"PostgreSQL 9.2.1",/'  SQLAlchemy-1.4.49/lib/sqlalchemy/dialects/postgresql/base.py
    cd $path/SQLAlchemy-1.4.49/
    python3 setup.py install
    cd $path
    pip3 install --no-index --find-links=$path sqlalchemy-utils==0.40.0
    pip3 install --no-index --find-links=$path starlette==0.36.3
    pip3 install --no-index --find-links=$path pyvmomi==8.0.1.0.1
    pip3 install --no-index --find-links=$path uvicorn==0.20.0
    pip3 install --no-index --find-links=$path sqlalchemy-migrate==0.13.0
    pip3 install --no-index --find-links=$path watchdog==3.0.0
    pip3 install --no-index --find-links=$path cryptography==43.0.1
    pip3 install --no-index --find-links=$path torch==v2.3.1
    pip3 install --no-index --find-links=$path numpy==1.25.2
    pip3 install --no-index --find-links=$path pandas==1.5.3
    pip3 install --no-index --find-links=$path joblib==1.2.0
    pip3 install --no-index --find-links=$path scikit-learn==1.5.1

}

SCRIPT_DIR=$(cd $(dirname $0) && pwd)
echo  $SCRIPT_DIR
echo  $1
if [ "$1" = "dev" ]; then
    main $SCRIPT_DIR dev
else
    main $SCRIPT_DIR prd
fi
