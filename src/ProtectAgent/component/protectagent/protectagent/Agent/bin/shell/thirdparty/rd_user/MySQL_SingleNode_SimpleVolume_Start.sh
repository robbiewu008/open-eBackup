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
. "$2/bin/agent_thirdpartyfunc.sh"

function GetSnapshotWWNsByLunId()
{
    AGENT_SRCLUNID=$1
    LUNID_POSTION=0
    SRCLUNID_ARRAY=(${AGENT_SRCLUNIDS//,/ })
    for varInfo in ${SRCLUNID_ARRAY[@]}
    do
        LUNID_POSTION=`expr $LUNID_POSTION + 1`
        if [ $varInfo -eq $AGENT_SRCLUNID ]
        then
            break
        fi
    done
    WWNINDEX=`expr $LUNID_POSTION - 1`
    MYSQL_LUN_WWN=${SNAPSHOTWWNS_ARRAY[$WWNINDEX]}
}


function MountDevice()
{
    SNAPSHOTWWNS_ARRAY=(${AGENT_SNAPSHOTWWNS//,/ })
    LUN_ID_PATH_ARRAY=(${LunIdMountPath//###/ })
    for lunIdPathInfo in ${LUN_ID_PATH_ARRAY[@]}
    do
        MYSQL_LUN_ID=` echo $lunIdPathInfo | ${MYAWK} -F "," '{print $1}'`
        MYSQL_LUN_Mount_PATH=`echo $lunIdPathInfo | ${MYAWK} -F "," '{print $2}'`
        GetSnapshotWWNsByLunId $MYSQL_LUN_ID
        #####sleep 15 sed to multipath scan,maybe the device not exit
        sleep 15
        FILE_SYSTEM_MOUNT_POINT=`multipath -ll | grep $MYSQL_LUN_WWN | ${MYAWK} -F " " '{print $1}'`
        mount /dev/mapper/$FILE_SYSTEM_MOUNT_POINT $MYSQL_LUN_Mount_PATH >> "${LOG_FILE_NAME}" 2>&1
        if [ $? -ne 0 ]
        then
            Log "[ERROR]:mount device failed."
            exit 1
        fi
        
        rm -rf $MYSQL_LUN_Mount_PATH/*.pid
        rm -rf $MYSQL_LUN_Mount_PATH/*.sock
    done
}

function Main()
{
    Log "[INFO]:Begin to start mysql." 
    
    which mysql
    if [ $? -ne 0 ]
    then
        Exit 1 -log "[INFO]:mysql is not isInstall." -ret "mysql is not isInstall"
    fi
    service mysql status | grep "not running" >>"${LOG_FILE_NAME}" 2>&1
    ret1=$?
    service mysqld status | grep "dead" >>"${LOG_FILE_NAME}" 2>&1
    ret2=$?
    if [[ $ret1 -ne 0 && $ret2 -ne 0 ]]
    then
        Exit 0 -log "[INFO]:mysql is already started."
    fi  

    
    GetValue "${INPUT_PARAMETER_LIST}" MountInfo
    LunIdMountPath=$ArgValue    
    if [ "$LunIdMountPath" = "" ]
    then
        Exit 1 -log "[ERROR]:LUN_ID_PATH configure failed." -ret "[ERROR]:LUN_ID_PATH configure failed."
    fi
    Log  $LunIdMountPath
    
    GetValue "${INPUT_PARAMETER_LIST}" SrcLunIds
    AGENT_SRCLUNIDS=$ArgValue 
    if [ "$AGENT_SRCLUNIDS" = "" ]
    then
        Exit 1 -log "[ERROR]:SrcLunIds failed." -ret "[ERROR]:SrcLunIds failed."
    fi 
    Log $AGENT_SRCLUNIDS
    
    GetValue "${INPUT_PARAMETER_LIST}" SnapshotCopyWWNs  
    AGENT_SNAPSHOTWWNS=$ArgValue  
    if [ "$AGENT_SNAPSHOTWWNS" = "" ]
    then
        Exit 1 -log "[ERROR]:SnapshotCopyWWNs failed." -ret "[ERROR]:SnapshotCopyWWNs failed."
    fi 
    Log $AGENT_SNAPSHOTWWNS    
    
    sh ${AGENT_ROOT_PATH}/bin/scandisk.sh ${AGENT_ROOT_PATH} >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Exit 1 -log "[ERROR]:Excute scan disk failed." -ret "[ERROR]:Excute scan disk failed."
    fi
    
    MountDevice
    
    [ -f ${TEMP_FILE_NAME} ] && rm -rf ${TEMP_FILE_NAME}
    
    mysqld_safe & service mysqld start >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Exit 1 -log "[ERROR]:Start mysql failed." -ret "[ERROR]:Start mysql failed."
    fi  
    

    Exit 0 -log "[INFO]:Finish start mysql." 
}
Main