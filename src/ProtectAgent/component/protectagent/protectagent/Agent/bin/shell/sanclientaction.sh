#!/bin/sh
# 
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
set +x
#!/usr/bin/expect

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/sanclientaction.log"
 
WHITE_LIST=(
"^/mnt/databackup/.*/.*$"
)

CheckWhiteList() {
    if [ $# -ne 1 ]; then
        Log "The number of paths to be verified is incorrect!"
        return 1
    fi
    InputPath="$1"
    AbsolutePath=`realpath ${InputPath}`
    Log "AbsolutePath:${AbsolutePath}"
    for wdir in "${WHITE_LIST[@]}"
    do
        if [[ $AbsolutePath =~ $wdir ]]; then
            return 0
        fi
    done
    return 1
}

# Log "********************************Start to execute the create lun********************************"

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

#for GetValue
TaskType=`GetValue "${PARAM_CONTENT}" tasktype`
SanclientWwpn=`GetValue "${PARAM_CONTENT}" sanclientwwpn`
AgentWwpn=`GetValue "${PARAM_CONTENT}" agentwwpn`
FileioName=`GetValue "${PARAM_CONTENT}" fileioname`
FileSystemSize=`GetValue "${PARAM_CONTENT}" filesystemsize`
FileioFullPathName=`GetValue "${PARAM_CONTENT}" fileiofullpathname`
Lunid=`GetValue "${PARAM_CONTENT}" lunid`
Filesystemmountpath=`GetValue "${PARAM_CONTENT}" filesystemmountpath`
Taskid=`GetValue "${PARAM_CONTENT}" taskid`
Sanclientwwpn=`echo ${SanclientWwpn} | sed -e s/0x// -e 's/../&:/g' -e s/:$//`
Agentwwpn=`echo ${AgentWwpn} | sed -e s/0x// -e 's/../&:/g' -e s/:$//`
FileIO=`targetcli /backstores/fileio ls`
FileCreate=`echo $FileIO | grep "$FileioName"`

# *************************** functions **********************************
function CreateLun0()
{
    LUN0_NAME="lun0.backend.fileio"
    LUN0_FILE="/dev/${LUN0_NAME}"
    LUN0_SIZE="10M"
    LUN0_FILEIO_PATH="/backstores/fileio/${LUN0_NAME}"
    LUN0_PATH="/qla2xxx/${Sanclientwwpn}/luns"

    # 1. create backend fileio
    Log "INFO: check whether ${LUN0_FILEIO_PATH} exists or not"
    targetcli ls "${LUN0_FILEIO_PATH}" | grep "${LUN0_FILE}" >> ${LOG_FILE_NAME}
    if [ $? -ne 0 ]; then
        Log "INFO: create backend file - ${LUN0_FILEIO_PATH} at ${LUN0_FILE}"
        targetcli /backstores/fileio/ create name=${LUN0_NAME} file_or_dev=${LUN0_FILE} size=${LUN0_SIZE} buffered=false >> ${LOG_FILE_NAME} 2>&1
        if [ $? -ne 0 ]; then
            Log "ERROR: failed to create fileio ${LUN0_NAME}."
            return 1
        fi
    else
        Log "INFO: lun0 backend fileio already exists"
    fi

    # 2. create lun0
    Log "INFO: check whether ${LUN0_PATH}/lun0 exists or not"
    targetcli ls "${LUN0_PATH}/lun0" | grep "${LUN0_FILE}" >> ${LOG_FILE_NAME}
    if [ $? -ne 0 ]; then
        Log "INFO: create lun0 at ${LUN0_PATH} from backend fileio ${LUN0_FILEIO_PATH}"
        targetcli ${LUN0_PATH} create ${LUN0_FILEIO_PATH} lun=0 >> ${LOG_FILE_NAME} 2>&1
        if [ $? -ne 0 ]; then
            Log "ERROR: failed to create lun0."
            return 1
        fi
    else
        Log "INFO: lun0 already exists"
    fi
    Log "INFO: create lun0 success"
    return 0
}

function CleanUpAndExitWhenLunCreationFailed()
{
    targetcli /backstore/fileio/ delete ${FileioName}
    #1为备份任务，2为恢复任务，恢复任务时不允许删除文件
    if [ ${TaskType} = 1 ]; then
        CheckWhiteList ${FileioFullPathName}
        if [ $? -ne 0 ]; then
            Log "ERROR(rm): Dir[${FileioFullPathName}] not in whitelist."
            return 1
        fi
        rm -rf ${FileioFullPathName}
    fi
    Log "Create Lun failed!"
    return 1
}


# *************************** main **********************************
Log "Tasktype is ${TaskType}."

mkdir ${Filesystemmountpath}/${Taskid}

#取消acls自动映射
targetcli set global auto_add_mapped_luns=false

if [ "$FileCreate" == "" ]; then
    if [ ${TaskType} = 1 ]; then
        #1为备份任务，2为恢复任务
        mkdir ${Filesystemmountpath}/${Taskid}
        targetcli /backstores/fileio/ create name=${FileioName} file_or_dev=${FileioFullPathName} size=${FileSystemSize} buffered=false  >> ${LOG_FILE_NAME} 2>&1
    elif [ ${TaskType} = 2 ] || [ ${TaskType} = 8 ]; then
        if [ -f "${FileioFullPathName}" ]; then
            Log "${FileioFullPathName} is exist."
            targetcli /backstores/fileio/ create name=${FileioName} file_or_dev=${FileioFullPathName} >> ${LOG_FILE_NAME} 2>&1
        else
            Log "${FileioFullPathName} is not exist.create fileio failed."
            exit 1
        fi
    else
        Log "Unsupport job type!"
        exit 1
    fi
    if [ $? -ne 0 ]; then
        Log "Create fileio failed!"
        exit 1
    fi
fi

#创建qla2xx通道
targetcli qla2xxx/ create ${Sanclientwwpn} >>${LOG_FILE_NAME} 2>&1
if [ $? -ne 0 ]; then
    targetcli /backstore/fileio/ delete ${FileioName}
    #1为备份任务，2为恢复任务，恢复任务时不允许删除文件
    if [ ${TaskType} = 1 ]; then
        CheckWhiteList ${FileioFullPathName}
        if [ $? -ne 0 ]; then
            Log "ERROR(rm): Dir[${FileioFullPathName}] not in whitelist."
            exit 1
        fi
        rm -rf ${FileioFullPathName}
    fi
    Log "Create qla2xxx failed!"
    exit 1
fi

# Create LUN0
# We must have a LUN 0 as per SCSI SAM specification requirements,
# otherwise AIX initiators may encounter scanning issue.
CreateLun0
if [ $? -ne 0 ]; then
    CleanUpAndExitWhenLunCreationFailed
    exit $?
fi

#执行命令创建lun
if [ "$FileCreate" == "" ]; then
    targetcli qla2xxx/${Sanclientwwpn}/luns/ create /backstores/fileio/${FileioName} lun=${Lunid} >>${LOG_FILE_NAME} 2>&1
    if [ $? -ne 0 ]; then
        CleanUpAndExitWhenLunCreationFailed
        exit $?
    fi
fi

#创建白名单
targetcli qla2xxx/${Sanclientwwpn}/acls/ create ${Agentwwpn} >>${LOG_FILE_NAME} 2>&1
if [ $? -ne 0 ]; then
    targetcli qla2xxx/${Sanclientwwpn}/luns/ delete lun${Lunid}
    targetcli /backstore/fileio/ delete ${FileioName}
    #1为备份任务，2为恢复任务，恢复任务时不允许删除文件
    if [ ${TaskType} = 1 ]; then
        CheckWhiteList ${FileioFullPathName}
        if [ $? -ne 0 ]; then
            Log "ERROR(rm): Dir[${FileioFullPathName}] not in whitelist."
            exit 1
        fi
        rm -rf ${FileioFullPathName}
    fi
    Log "Create agentwwpn failed!"
    exit 1
fi

#添加acls映射
targetcli qla2xxx/${Sanclientwwpn}/acls/${Agentwwpn} create mapped_lun=0 tpg_lun=0 >>${LOG_FILE_NAME} 2>&1
targetcli qla2xxx/${Sanclientwwpn}/acls/${Agentwwpn} create mapped_lun=${Lunid} tpg_lun=${Lunid} >>${LOG_FILE_NAME} 2>&1
if [ $? -ne 0 ]; then
    CleanUpAndExitWhenLunCreationFailed
    exit $?
fi

#保存配置，嵌入式使用expect免交互
/usr/bin/expect <<-EOF
spawn targetcli saveconfig
expect "Type"
send "yes\r"
expect eof
exit 0
EOF