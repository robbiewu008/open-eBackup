#!/bin/bash

########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################

backup_flag=/usr/local/gaussdb/backup_flag
G_SUDO_SCRIPT_PATH=/opt/script

function check_db_state() {
    local dbinfo=$(export LD_LIBRARY_PATH=/usr/local/gaussdb/app/lib;/usr/local/gaussdb/app/bin/gs_ctl -D /usr/local/gaussdb/data query -U GaussDB)
    local db_state=""
    eval $(echo "$dbinfo" | grep -Ew "db_state"           | sed -e 's/:/=/g' | sed -e 's/ //g' | sed -n '1p')
    # 检查gaussdb状态是否为Normal
    if [ "${db_state}" == "Normal" ]; then
        return 0
    fi
    return 1
}

function main()
{
    if [ -f "${backup_flag}" ];then
        # 升级过程中，备份和恢复gaussdb数据过程中不监听gaussdb端口
        return 0
    fi

    # Lun异常状态时gs_ctl返回时间长，防止gs_ctl进程启动太多，添加数量限制
    local gs_ctl_num=$(pidof gs_ctl | wc -w)
    if [ $gs_ctl_num -gt 5 ]; then
        # gs_ctl 数量为3时，说明lun状态异常，返回异常
        return 1
    fi
    check_db_state
}

main