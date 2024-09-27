#!/bin/sh
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
set +x
#!/usr/bin/expect

#@function: mount nas share path
 
AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
 
#for log
LOG_FILE_NAME="${LOG_PATH}/sanclientaction.log"

BLOCK_LIST="^/$\|^/bin$\|^/bin/.*\|^/boot$\|^/boot/.*\|^/dev$\|^/dev/.*\|^/etc$\|^/etc/.*\|^/lib$\|^/lib/.*\|^/lost+found$\|^/lost+found/.*\|^/media$\|^/media/.*\|^/mnt$\|^/opt$\|^/proc$\|^/proc/.*\|^/root$\|^/root/.*\|^/sbin$\|^/sbin/.*\|^/selinux$\|^/selinux/.*\|^/srv$\|^/srv/.*\|^/sys$\|^/sys/.*\|^/usr$\|^/usr/.*\|^/var$\|^/var/.*\|^/run$\|^/run/.*"
 
CheckMountPath()
{
    realMountPath=""
    if [ ${SYS_NAME} = "AIX" ] || [ ${SYS_NAME} = "SunOS" ]; then
        cd $1
        realMountPath=`pwd`
        Log "realpath=${realMountPath}"
        cd -
    else
        mountPath=$1
        realMountPath=`realpath ${mountPath}`
    fi
    expr match ${realMountPath} ${BLOCK_LIST} > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The umount path is in the blocklist, path is $1."
        return 1
    fi
    return 0
}

Log "********************************Start to execute delete lun********************************"
 
# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"
 
#for GetValue
SanclientWwpn=`GetValue "${PARAM_CONTENT}" sanclientwwpn`
AgentWwpn=`GetValue "${PARAM_CONTENT}" agentwwpn`
FileioName=`GetValue "${PARAM_CONTENT}" fileioname`
FileioFullPath=`GetValue "${PARAM_CONTENT}" fileiofullpath`
Lunid=`GetValue "${PARAM_CONTENT}" lunid`
Sanclientwwpn=`echo ${SanclientWwpn} | sed -e s/0x// -e 's/../&:/g' -e s/:$//`
Agentwwpn=`echo ${AgentWwpn} | sed -e s/0x// -e 's/../&:/g' -e s/:$//`
SanclientPort=`GetValue "${PARAM_CONTENT}" sanclientPort`

SanclientIqn=${SanclientWwpn%_*}
SanclientIp=${SanclientWwpn#*_}

adapterType=`echo ${SanclientWwpn} | grep iqn`
if [ "${adapterType}" = "" ]; then
    adapterType="fc"
else
    adapterType="iscsi"
fi

if [ "${adapterType}" = "fc" ]; then
    #删除lun组
    targetcli qla2xxx/${Sanclientwwpn}/luns/ delete lun${Lunid} >>${LOG_FILE_NAME} 2>&1
    if [ $? -ne 0 ]; then
        errormessage=`cat ${LOG_FILE_NAME}`
        Log "${errormessage}"
    fi
 
else
    #删除lun组
    targetcli iscsi/${SanclientIqn}/tpg1/luns/ delete lun${Lunid} >> ${LOG_FILE_NAME} 2>&1
    if [ $? -ne 0 ]; then
        errormessage=`cat ${LOG_FILE_NAME}`
        Log "${errormessage}"
    fi
 
 
    #删除ip和端口
    targetcli iscsi/${SanclientIqn}/tpg1/portals/ delete ${SanclientIp} ${SanclientPort} >> ${LOG_FILE_NAME} 2>&1
    if [ $? -ne 0 ]; then
        errormessage=`cat ${LOG_FILE_NAME}`
        Log "${errormessage}"
    fi
fi

#删除文件
targetcli /backstores/fileio delete ${FileioName} >> ${LOG_FILE_NAME} 2>&1
if [ $? -ne 0 ]; then
    errormessage=`cat ${LOG_FILE_NAME}`
    Log "${errormessage}"
fi
 
CheckMountPath "${FileioFullPath}"
if [ $? -eq 0 ]; then
    umount -f ${FileioFullPath} >> ${LOG_FILE_NAME} 2>&1
    if [ $? -ne 0 ]; then
        errormessage=`cat ${LOG_FILE_NAME}`
        Log "${errormessage}"
    fi
fi

#保存配置
spawn targetcli saveconfig
expect "Type"
send "yes\r"
expect eof
exit 0