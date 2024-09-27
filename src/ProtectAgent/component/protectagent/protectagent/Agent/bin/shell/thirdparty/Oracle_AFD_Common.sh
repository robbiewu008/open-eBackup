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
AGENT_THIRDPARTY_BINPATH="${AGENT_ROOT_PATH}/bin"
AGENT_THIRDPARTY_TMPPATH="${AGENT_ROOT_PATH}/tmp"
AGENT_THIRDPARTY_LOGPATH="${AGENT_ROOT_PATH}/log"
AGENT_THIRDPARTY_RSTFILE="$AGENT_THIRDPARTY_TMPPATH/RST${PID}.txt"
AGENT_THIRDPARTY_INPUT_PARA_TEMP_FILE="${AGENT_THIRDPARTY_TMPPATH}/input_tmp${PID}"
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#for GetValue
ArgFile=$$

getSnapShotWWNByLunID()
{
    SOURCE_LUN_ID=$1
    
    PER_ITEMS=`echo ${PRE_SNAPSHOT} | sed -e 's/+/ /g'`
    for item in ${PER_ITEMS}
    do
        SNAPSHOT_NAME="${item}_${SOURCE_LUN_ID}_"
        # get wwn by config
        snapShotWWN=`upadmin show vlun type=all | grep "${SNAPSHOT_NAME}" | sed -n 1p | awk '{print $4}'`
        if [ ! -z "$snapShotWWN" ]
        then
            echo ${snapShotWWN}
            return
        fi
    done
}

getDiskNameByLunID()
{
    SOURCE_LUN_ID=$1
    
    PER_ITEMS=`echo ${PRE_SNAPSHOT} | sed -e 's/+/ /g'`
    for item in ${PER_ITEMS}
    do
        SNAPSHOT_NAME="${item}_${SOURCE_LUN_ID}_"
        # get wwn by config
        diskName=`upadmin show vlun type=all | grep "${SNAPSHOT_NAME}" | sed -n 1p | awk '{print $2}'`
        if [ ! -z "$diskName" ]
        then
            echo ${diskName}
            return
        fi
    done
}

configLun4Udev()
{
    SOURCE_LUN_ID=$1
    SOURCE_LUN_WWN=$2
    if [ -z "${SOURCE_LUN_ID}" ]
    then
        Log "source id is null."
        return 1
    fi

    if [ -z "$SOURCE_LUN_WWN" ]
    then
        Log "Can't get source lun WWN with source lun id ${SOURCE_LUN_ID}."
        return 1
    fi
    
    # get wwn by config
    snapShotWWN=`getSnapShotWWNByLunID ${SOURCE_LUN_ID}`
    if [ -z "$snapShotWWN" ]
    then
        Log "Can't get snapshot WWN with source lun id ${SOURCE_LUN_ID}."
        Log "`upadmin show vlun type=all `"
        return 1
    fi
    
    Log "Begin to config udev wwn=$snapShotWWN."
    # config udev
    configUDEV $SOURCE_LUN_WWN $snapShotWWN
    lastErr=$?
    if [ $lastErr -ne 0 ]
    then
        Log "config udev failed."
        return 1
    fi
}

## change udeve config
configUDEV()
{
    local sourceWWN=$1
    local dstWWN=$2
    
    # get source wwn udev config
    local rule_udev=`cat $UDEV_FILE | grep "3${sourceWWN}"`
    
    if [ -z "${rule_udev}" ]
    then
        Log "udev file ${UDEV_FILE}, can't find ${sourceWWN}."
        Log "`cat ${UDEV_FILE}`"
        return 1
    fi
    
    # replace config
    local new_rule=`echo ${rule_udev} | sed "s/${sourceWWN}/${dstWWN}/g"`
    
    # write new rules
    echo "${new_rule}" >> "${UDEV_FILE}"
}

## change udeve config
removeLunConfig()
{
    SOURCE_LUN_ID=$1
    if [ -z "${SOURCE_LUN_ID}" ]
    then
        Log "source id is null."
        return 1
    fi
    
    # get wwn by config
    snapShotWWN=`getSnapShotWWNByLunID ${SOURCE_LUN_ID}`
    if [ -z "$snapShotWWN" ]
    then
        Log "Can't get snapshot WWN with source lun id ${SOURCE_LUN_ID}."
        Log "`upadmin show vlun type=all`"
        return 0
    fi

    # remove udev config
    Log "Begin to remove udev wwn=$snapShotWWN."
    sed -i "/3${snapShotWWN}/d" $UDEV_FILE
    lastErr=$?
    if [ $lastErr -ne 0 ]
    then
        Log "remove config udev file failed."
        return 1
    fi
}

## call agent shell to start db
startDB()
{
    local AGENT_ROOT_PATH=$1
    if [ -z "${AGENT_ROOT_PATH}" ]
    then
        Log "start: agent root is null."
        return 1
    fi
    
    local PARAM_PID=stardb$$
    local PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${PARAM_PID}"
    
    # check param file exists
    if [ -f "${PARAM_FILE}" ]
    then
        Log "${PARAM_FILE} is exists."
        Log "`cat ${PARAM_FILE}`"
        rm -rf "${PARAM_FILE}"
    fi
    
    touch ${PARAM_FILE}
    # write params
    local INPUT_INFO="InstanceName=${InstanceName}:AppName=${AppName}:UserName=${UserName}:Password=${Password}:Action=${ACTION_START}:IsASM=${IsASM}:ASMDiskGroups=${ASMDiskGroups}:ASMUserName=${ASMUserName}:ASMPassword=${ASMPassword}:ASMInstanceName=${ASMInstanceName}:OracleHome=:IS_INCLUDE_ARCH="
    echo "$INPUT_INFO" > ${PARAM_FILE}
    
    "${AGENT_ROOT_PATH}/bin/oradbaction.sh" "${AGENT_ROOT_PATH}" "${PARAM_PID}"
    return $?
}

## call agent shell to stop db
stopDB()
{
    local AGENT_ROOT_PATH=$1
    if [ -z "${AGENT_ROOT_PATH}" ]
    then
        Log "stop: agent root is null."
        return 1
    fi
    
    local PARAM_PID=stopdb$$
    local PARAM_FILE="${AGENT_ROOT_PATH}/tmp/input_tmp${PARAM_PID}"
    
    # check param file exists
    if [ -f "${PARAM_FILE}" ]
    then
        Log "${PARAM_FILE} is exists."
        Log "`cat ${PARAM_FILE}`"
        rm -rf "${PARAM_FILE}"
    fi
    
    touch ${PARAM_FILE}
    # write params
    local INPUT_INFO="InstanceName=${InstanceName}:AppName=${AppName}:UserName=${UserName}:Password=${Password}:Action=${ACTION_STOP}:IsASM=${IsASM}:ASMDiskGroups=${ASMDiskGroups}:ASMUserName=${ASMUserName}:ASMPassword=${ASMPassword}:ASMInstanceName=${ASMInstanceName}:OracleHome=:IS_INCLUDE_ARCH="
    echo "$INPUT_INFO" > ${PARAM_FILE}
    
    "${AGENT_ROOT_PATH}/bin/oradbaction.sh" "${AGENT_ROOT_PATH}" "${PARAM_PID}"
    return $?
}

# unlabel afd disk
clearAFDDisk()
{
    su - ${ASM_USER} -c "asmcmd afd_scan"
    lastErr=$?
    if [ $lastErr -ne 0 ]
    then
        Exit 1 -log "afd_scan failed, code=${lastErr}." -ret ${lastErr}
    fi
    
    AFD_DISKS=`echo ${LABEL_LIST} | sed -e 's/;/ /g'`
    for item in ${AFD_DISKS}
    do
        label=`echo ${item} | awk -F "&&" '{print $2}'`
        
        su - ${ASM_USER} -c "asmcmd afd_lsdsk" | grep "^${label} "
        if [ $? -eq 0 ]
        then
            su - ${ASM_USER} -c "asmcmd afd_unlabel ${label} -f"
            lastErr=$?
            if [ $lastErr -ne 0 ]
            then
                Exit 1 -log "afd_unlabel ${label} failed, code=${lastErr}." -ret ${lastErr}
            fi
        else
            Log "${label} is not exists."
        fi
    done
}

refreshAFDDisk()
{
    su - ${ASM_USER} -c "asmcmd afd_refresh"
    lastErr=$?
    if [ $lastErr -ne 0 ]
    then
        Exit 1 -log "afd_refresh failed, code=${lastErr}." -ret ${lastErr}
    fi
}

scanAFDDisk()
{
    su - ${ASM_USER} -c "asmcmd afd_scan"
    lastErr=$?
    if [ $lastErr -ne 0 ]
    then
        Exit 1 -log "afd_scan failed, code=${lastErr}." -ret ${lastErr}
    fi
    
    # if not afd label, label new one
    bLabel=0
    AFD_DISKS=`echo ${LABEL_LIST} | sed -e 's/;/ /g'`
    for item in ${AFD_DISKS}
    do
        disk=`echo ${item} | awk -F "&&" '{print $1}'`
        label=`echo ${item} | awk -F "&&" '{print $2}'`
        
        su - ${ASM_USER} -c "asmcmd afd_lsdsk" | grep "^${label} "
        if [ $? -ne 0 ]
        then
            Log "${label} is not exists."
            if [ "${AFD_MODE}" = "1" ]
            then
                lableAFDDisk ${disk} ${label}
            elif [ "${AFD_MODE}" = "0" ]
            then
                lableAFDDisk4Native ${disk} ${label}
            fi
        else
            Log "${label} is exists."
        fi
    done

    # scan agagin, some time after label the afd disk maybe disapper
    su - ${ASM_USER} -c "asmcmd afd_scan"
    lastErr=$?
    if [ $lastErr -ne 0 ]
    then
        Exit 1 -log "afd_scan failed, code=${lastErr}." -ret ${lastErr}
    fi
}

lableAFDDisk()
{
    disk=$1
    label=$2
    su - ${ASM_USER} -c "asmcmd afd_label ${label} ${disk} --rename" >> ${LOG_FILE_NAME} 2>&1
    lastErr=$?
    if [ $lastErr -ne 0 ]
    then
        Exit 1 -log "adf_label ${item} failed, code=${lastErr}." -ret ${lastErr}
    fi
    Log "Lable ${label} success."
}

lableAFDDisk4Native()
{
    diskInfo=$1
    label=$2
    srcLUNID=`echo ${diskInfo} | awk -F ":" '{print $1}'`
    partition=`echo ${diskInfo} | awk -F ":" '{print $2}'`
    
    # plan using srouce LUN ID directly to find disk name
    diskName=`upadmin show vlun type=all | grep " ${srcLUNID} " | sed -n 1p | awk '{print $2}'`
    # test using snapshot to find disk name
    if [ -z "${diskName}" ]
    then
        diskName=`getDiskNameByLunID ${srcLUNID}`
    fi
    
    if [ -z "${diskName}" ]
    then
        Exit 1 -log "diskName is not exists, diskInfo=${diskInfo}, label=${label}."
    fi
    
    if [ -e "/dev/${diskName}${partition}" ]
    then
        Log "asmcmd afd_label ${label} /dev/${diskName}${partition} --rename"
        su - ${ASM_USER} -c "asmcmd afd_label ${label} /dev/${diskName}${partition} --rename" >> ${LOG_FILE_NAME} 2>&1
        lastErr=$?
        if [ $lastErr -ne 0 ]
        then
            Exit 1 -log "adf_label ${diskInfo} failed, code=${lastErr}." -ret ${lastErr}
        fi
        Log "Lable ${label} success."
    else
        Exit 1 -log "disk /dev/${diskName}${partition} is not exists, ${diskInfo}." -ret ${lastErr}
    fi
}

configLun4Native()
{
    SOURCE_LUN_ID=$1
    SOURCE_LUN_WWN=$2
    if [ -z "${SOURCE_LUN_ID}" ]
    then
        Log "source id is null."
        return 1
    fi
    
    if [ -z "$SOURCE_LUN_WWN" ]
    then
        Log "Can't get source lun WWN with source lun id ${SOURCE_LUN_ID}."
        return 1
    fi
    
    # get wwn by config
    diskName=`getDiskNameByLunID ${SOURCE_LUN_ID}`
    Log "get diskname ${diskName} by source lun id ${SOURCE_LUN_ID}."
    
    if [ ! -z "${diskName}" ]
    then
        chmod ${AFD_NATIVE_MODE} /dev/${diskName}*
        chown -h ${ASM_USER}:${AFD_DISK_GROUP} /dev/${diskName}*
    else
        Log "have no disk with source lun id ${SOURCE_LUN_ID}."
    fi
}

configLun4NativePlan()
{
    SOURCE_LUN_ID=$1
    SOURCE_LUN_WWN=$2

    if [ -z "${SOURCE_LUN_ID}" ]
    then
        Log "source id is null."
        return 1
    fi
    
    if [ -z "$SOURCE_LUN_WWN" ]
    then
        Log "Can't get source lun WWN with source lun id ${SOURCE_LUN_ID}."
        return 1
    fi
    
    # get wwn by config
    diskName=`upadmin show vlun type=all | grep "${SOURCE_LUN_WWN}" | sed -n 1p | awk '{print $2}'`
    
    Log "get diskname ${diskName}-${SOURCE_LUN_WWN}."
    
    if [ ! -z "${diskName}" ]
    then
        chmod ${AFD_NATIVE_MODE} /dev/${diskName}*
        chown -h ${ASM_USER}:${AFD_DISK_GROUP} /dev/${diskName}*
    else
        Log "have no disk ${SOURCE_LUN_WWN}."
    fi
}

