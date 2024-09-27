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
#@function: mount nas share path

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#********************************define these for local script********************************
#for log
LOG_FILE_NAME="${LOG_PATH}/umountnasmedia.log"
#for GetValue

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

serviceType=`GetValue "${PARAM_CONTENT}" serviceType`
StorageIP=`GetValue "${PARAM_CONTENT}" storageIp`

test "$serviceType" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "serviceType"
test "$StorageIP" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "StorageIP"

storIPs=`echo ${StorageIP} | sed 's/;/ /g'`

if [ "${serviceType}" = "vmware_fc" ]; then
     # vmware feature usage
    Log "in vmware_fc branch."
    nasFileSystemName=`GetValue "${PARAM_CONTENT}" nasFileSystemName`
    BackupID=`GetValue "${PARAM_CONTENT}" backupID`
    ParentTaskID=`GetValue "${PARAM_CONTENT}" parentTaskID`
    test "$nasFileSystemName" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "nasFileSystemName"
    test "$BackupID" = "${ERROR_PARAM_INVALID}"                              && ExitWithError "BackupID"
    test "$ParentTaskID" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ParentTaskID"
    Log "PID=${PID};nasFileSystemName=${nasFileSystemName};ParentTaskID=${ParentTaskID};BackupID=${BackupID}."
    test -z "$BackupID"        && ExitWithError "BackupID"
    MountPointPrefix="/opt/advbackup/vmware/data"
    MountPointUmounted="${MountPointPrefix}/${BackupID}"
    if [ ! -z "${nasFileSystemName}" ]; then
        NasFileSystemUmounted="/${nasFileSystemName}"
        Log "Task[${ParentTaskID}:${BackupID}], target nas filesystem: ${NasFileSystemUmounted}"
    fi
    MountPointed=`mount | grep "${nasFileSystemName}" | awk '{print $3}' | grep "${MountPointUmounted}"`
    if [ $? -eq 0 ]; then
        umount -vl ${MountPointed} >> $LOG_FILE_NAME 2>&1
        if [ $? -ne 0 ]; then
            Log "Task[${ParentTaskID}:${BackupID}], umount nas mountpoint: ${MountPointUmounted} failed, coresponding nas filesystem: ${NasFileSystemUmounted}."
            cd ${MountPointPrefix}/../
            umount -f ${MountPointed}
            if [ $? -ne 0 ]; then
                Log "Task[${ParentTaskID}:${BackupID}], force umount nas mountpoint: ${MountPointUmounted} failed, suggest to delete it manually."
            fi
        else
            Log "Task[${ParentTaskID}:${BackupID}], umount nas mountpoint: ${MountPointUmounted} successfully, coresponding nas filesystem: ${NasFileSystemUmounted}!"
            # delete mnt only when umount successfully
            chattr -i ${MountPointed}
            rm -rf ${MountPointUmounted}
            if [ ! -d "${MountPointUmounted}" ]; then
                Log "Task[${ParentTaskID}:${BackupID}], delete mountpoint: ${MountPointUmounted} successfully!"
            else
                Log "Task[${ParentTaskID}:${BackupID}], Unable to delete mountpoint: ${MountPointUmounted}, suggest to delete it manually!"
            fi
        fi
    else
        Log "Task[${ParentTaskID}:${BackupID}], target nas filesystem mountpoint ${MountPointUmounted} does not exist, will skip umount operation!"
    fi
    exit 0
fi

if [ "${serviceType}" = "vmware" ]; then
    # vmware feature usage
    nasFileSystemName=`GetValue "${PARAM_CONTENT}" nasFileSystemName`
    BackupID=`GetValue "${PARAM_CONTENT}" backupID`
    ParentTaskID=`GetValue "${PARAM_CONTENT}" parentTaskID`

    test "$nasFileSystemName" = "${ERROR_PARAM_INVALID}"                     && ExitWithError "nasFileSystemName"
    test "$BackupID" = "${ERROR_PARAM_INVALID}"                              && ExitWithError "BackupID"
    test "$ParentTaskID" = "${ERROR_PARAM_INVALID}"                          && ExitWithError "ParentTaskID"

    Log "PID=${PID};nasFileSystemName=${nasFileSystemName};StorageIP=${StorageIP};ParentTaskID=${ParentTaskID};BackupID=${BackupID}."
    test -z "$StorageIP"       && ExitWithError "StorageIP"
    test -z "$BackupID"        && ExitWithError "BackupID"

    MountPointPrefix="/opt/advbackup/vmware/data"
    MountPointUmounted="${MountPointPrefix}/${BackupID}"
    if [ ! -z "${nasFileSystemName}" ]; then
        for ip in ${storIPs}; do
            NasFileSystemUmounted="${ip}:/${nasFileSystemName}"
            Log "Task[${ParentTaskID}:${BackupID}], target nas filesystem: ${NasFileSystemUmounted}"
            # only umount mnt exists
            MountPointed=`mount | grep "${nasFileSystemName}" | awk '{print $3}' | grep "${MountPointUmounted}"`
            if [ $? -eq 0 ]; then
                umount -vl ${MountPointed} >> $LOG_FILE_NAME 2>&1
                if [ $? -ne 0 ]; then
                    Log "Task[${ParentTaskID}:${BackupID}], umount nas mountpoint: ${MountPointUmounted} failed, coresponding nas filesystem: ${NasFileSystemUmounted}."
                    cd ${MountPointPrefix}
                    cd ..
                    umount -f ${MountPointed}
                    if [ $? -ne 0 ]; then
                        Log "Task[${ParentTaskID}:${BackupID}], force umount nas mountpoint: ${MountPointUmounted} failed, suggest to delete it manually."
                    fi
                else
                    Log "Task[${ParentTaskID}:${BackupID}], umount nas mountpoint: ${MountPointUmounted} successfully, coresponding nas filesystem: ${NasFileSystemUmounted}!"
                    # delete mnt only when umount successfully
                    chattr -i ${MountPointed}
                    rm -rf ${MountPointUmounted}
                    if [ ! -d "${MountPointUmounted}" ]; then
                        Log "Task[${ParentTaskID}:${BackupID}], delete mountpoint: ${MountPointUmounted} successfully!"
                    else
                        Log "Task[${ParentTaskID}:${BackupID}], Unable to delete mountpoint: ${MountPointUmounted}, suggest to delete it manually!"
                    fi      
                fi
            else
                Log "Task[${ParentTaskID}:${BackupID}], target nas filesystem mountpoint ${MountPointUmounted} does not exist, will skip umount operation!"
            fi
        done
    else
        Log "Task[${ParentTaskID}:${BackupID}], target nas filesystem is not provided, please check!"
        exit 1
    fi
elif [ "${serviceType}" = "archivestream" ]; then
    # archivestream feature usage
    LocalPath=`GetValue "${PARAM_CONTENT}" LocalPath`
    test "$LocalPath" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "LocalPath"
    
    Log "PID=${PID};LocalPath=${LocalPath}."

    if [ ! -z "${LocalPath}" ]; then
        # only umount mnt exists
        mount | grep "${LocalPath}"
        if [ $? -eq 0 ]; then
            WhiteList="^/mnt/databackup/archivestream/.*/.*$"
            expr match ${LocalPath} ${WhiteList} > /dev/null 2>&1
            if [ $? -ne 0 ]; then
                Log "umount failed: ${LocalPath} is not in white list."
                exit 1
            fi
            if [ -L ${LocalPath} ]; then
                Log "umount failed: ${LocalPath} is soft link folder."
                exit 1
            fi
            umount -vl ${LocalPath} >> $LOG_FILE_NAME 2>&1
            if [ $? -ne 0 ]; then
                Log "umount nas mountpoint: ${LocalPath} failed."
            else
                Log "umount nas mountpoint: ${LocalPath} successfully!"
                # delete mnt only when umount successfully
                FatherPath=`dirname ${LocalPath}`
                rmdir ${LocalPath} >> $LOG_FILE_NAME 2>&1
                rmdir ${FatherPath} >> $LOG_FILE_NAME 2>&1
                if [ ! -d "${LocalPath}" ]; then
                    Log "delete mountpoint: ${LocalPath} successfully!"
                else
                    Log "Unable to delete mountpoint: ${LocalPath}, suggest to delete it manually!"
                fi
            fi
        else
            Log "target nas filesystem mountpoint ${LocalPath} does not exist, will skip umount operation!"
        fi
    else
        Log "target nas filesystem is not provided, please check!"
        exit 1
    fi
else
    Log "Invalid service type: ${serviceType}."
    exit 1
fi

exit 0
