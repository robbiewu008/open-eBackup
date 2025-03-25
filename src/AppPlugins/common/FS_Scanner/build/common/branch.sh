#!/bin/bash
# 当前工程使用的所有版本信息
if [ -z "${branch}" ];then
    SCANNER_BRANCH=develop_backup_software_1.6.0RC1
else
    SCANNER_BRANCH=${branch}
fi
echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO]  Current SCANNER_BRANCH is ${SCANNER_BRANCH}"

# MODULE的代码仓分支
if [ -z "${MODULE_BRANCH}" ]; then
    if [ -z "${branch}" ];then
        MODULE_BRANCH=develop_backup_software_1.6.0RC1
    else
        MODULE_BRANCH=${branch}
    fi
fi
echo "[$(date "+%Y-%m-%d %H:%M:%S")][INFO]  Current MODULE_BRANCH is ${MODULE_BRANCH}"
