#!/bin/sh
set +x
#@function: mount dataturbo share path

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
umask 0022
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
sysName=`uname -s`

#********************************define these for local script********************************
#for log
LOG_FILE_NAME="${LOG_PATH}/preparedataturbomedia.log"

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

serviceType=`GetValue "${PARAM_CONTENT}" serviceType`

test "$serviceType" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "serviceType"

# mark netwrok connection with nfs service
RetryTimes=3
ConnectFlag=0

CheckMountPoint()
{
    sharePath=$1
    MountPoint=$2
    
    Log "Mount point $MountPoint"
    Log "sharepath point $sharePath"

    //check mount Mount Point
    sourceFS=`mount | grep "${MountPoint}"`
    Log "sourceFS $sourceFS" 
    if [ $? -eq 0 -o -n "${sharePath}" ]; then
        sourceFS=`mount | grep "${MountPoint}" | $MYAWK '{print $1}' | grep "${sharePath}"`
        if [ $? -eq 0 -o -n "${sourceFS}" ]; then
            Log "Mounted ${sourceFS}, will skip mount operation."
            return 0
        fi
    fi
}

Mount()
{
    StorageName=$1
    sharePath=$2
    MountPoint=$3

    CheckMountPoint "$sharePath" "$MountPoint"
    if [ $? -ne 0 ]; then
        Log "check mount point ${sharePath} ${MountPoint} failed"
        return 1
    fi

    [ ! -d $MountPoint ] && mkdir -p $MountPoint
    # change mountpoint attr read-only
    chattr +i ${MountPoint}

    Log "dataturbo mount storage_object storage_name=${StorageName} filesystem_name=${sharePath} mount_dir=${MountPoint}"
    retryTime=1
    while [ $retryTime -le 3 ]; do
        retryTime=`expr $retryTime + 1`
        dataturbo mount storage_object storage_name="${StorageName}" filesystem_name="${sharePath}" mount_dir="${MountPoint}" >> $LOG_FILE_NAME 2>&1
        if [ $? -ne 0 ]; then
            Log "Mount ${StorageName} ${sharePath} ${MountPoint} failed retry $retryTime time"
            if [ $retryTime -ge 4 ]; then
                return 1
            fi
            sleep 5
            continue
        fi

        mount | grep ${MountPoint}
        if [ $? -eq 0 ]; then
            Log "Mount ${StorageName} ${sharePath} ${MountPoint} success."
            return 0
        fi

        Log "Mount  ${StorageName} ${FileSystemMountPath} ${HostMountPath} check failed retry $retryTime time"
        if [ $retryTime -ge 4 ]; then
            return 1
        fi
        sleep 5
    done
    
    Log "Mount ${StorageName} ${sharePath} ${MountPoint} failed."
    return 1
}


MountData()
{
    Log "Begin Mount data share path ${StorageName} ${DataSharePath} ${DataMountPath}"
    Mount "${StorageName}" "${DataSharePath}" "${DataMountPath}"
    if [ $? -eq 0 ]; then
        chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}
        if [ ! -d "${DataMountPath}/data" ]; then
            if [ -d "${DataMountPath}/additional" ]; then
                chown -h -R root:root ${DataMountPath}/additional
                chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}/additional/dbs
                chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}/additional/netadmin
            fi
        else
            if [ -d "${DataMountPath}/data/additional" ]; then
                chown -h -R root:root ${DataMountPath}/data/additional
                chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}/data/additional/dbs
                chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}/data/additional/netadmin
            fi                 
        fi
        mountDataPaths="${mountDataPaths}${DataMountPath};"
    fi
}

MountLog()
{
    Mount "${StorageName}" "${LogSharePath}" "${LogMountPath}"
    if [ $? -eq 0 ]; then
        mountLogPaths=${LogMountPath}
    fi
}

if [ "${serviceType}" = "database" ]; then
    GetOracleUser
    DataSharePath=`GetValue "${PARAM_CONTENT}" dataShareMountPath`
    LogSharePath=`GetValue "${PARAM_CONTENT}" logShareMountPath`
    DBUUID=`GetValue "${PARAM_CONTENT}" DBUUID`
    StorageName=`GetValue "${PARAM_CONTENT}" storageName`

    test "$DataSharePath" = "${ERROR_PARAM_INVALID}"                            && ExitWithError "DataSharePath"
    test "$LogSharePath" = "${ERROR_PARAM_INVALID}"                             && ExitWithError "LogSharePath"
    test "$DBUUID" = "${ERROR_PARAM_INVALID}"                                   && ExitWithError "DBUUID"
    test "$StorageName" = "${ERROR_PARAM_INVALID}"                              && ExitWithError "StorageName"

    DataMountPath="/tmp/advbackup/data/${DBUUID}/dataturbo"
    LogMountPath="/tmp/advbackup/log/${DBUUID}/dataturbo"

    mountDataPaths=
    mountLogPaths=
    DataSharePath=`echo ${DataSharePath%?}`
    LogSharePath=`echo ${LogSharePath%?}`
    Log "StorageName = $StorageName,LogSharePath = $LogSharePath, DataSharePath = $DataSharePath"
    DeleteFile ${RESULT_FILE}

    if [ ! -z "${DataSharePath}" ]; then
        Log "Mount Datashare path ${DataSharePath}"
        MountData 
        test -z "$mountDataPaths" && ExitWithErrorCode "dataturbo data path" ${ERROR_DISCONNECT_STORAGE_NETWORK}
    fi

    if [ ! -z "${LogSharePath}" ]; then
        Log "Mount LogShare Path ${LogSharePath}"
        MountLog
        test -z "$mountLogPaths" && ExitWithErrorCode "dataturbo log path" ${ERROR_DISCONNECT_STORAGE_NETWORK}
    fi

    echo ${mountDataPaths} > "${RESULT_FILE}"
    echo ${mountLogPaths} >> "${RESULT_FILE}"
    Log "Mount oracle dataturbo success."
elif [ "${serviceType}" = "vmware" ]; then
    NasFileSystemName=`GetValue "${PARAM_CONTENT}" nasFileSystemName`
    StorageName=`GetValue "${PARAM_CONTENT}" storageName`

    MountPointPrefix="/opt/advbackup/vmware/data"
    MountPointExpected="${MountPointPrefix}/${BackupID}"

    # parent task id
    BackupID=`GetValue "${PARAM_CONTENT}" backupID`
    ParentTaskID=`GetValue "${PARAM_CONTENT}" parentTaskID`

    test "$NasFileSystemName" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "NasFileSystemName"
    test "$BackupID" = "${ERROR_PARAM_INVALID}"                                && ExitWithError "BackupID"
    test "$ParentTaskID" = "${ERROR_PARAM_INVALID}"                            && ExitWithError "ParentTaskID"

    Log "PID=${PID};NasFileSystemName=${NasFileSystemName};ParentTaskID=${ParentTaskID};BackupID=${BackupID}."

    # generate nas filesystem mnt expected
    MountPointPrefix="/opt/advbackup/vmware/data"
    MountPointExpected="${MountPointPrefix}/${BackupID}"

    if [ ! -z "${NasFileSystemName}" ]; then
        # create mnt path
        if [ ! -d ${MountPointExpected} ]; then
            mkdir -p ${MountPointExpected} >> $LOG_FILE_NAME 2>&1
            chmod 755 "/opt/advbackup"
            chmod 700 ${MountPointExpected}
        fi
        Log "Task[${ParentTaskID}:${BackupID}], mountpoint expected: ${MountPointExpected}"

        NasFileSystemName="/${NasFileSystemName}"
        Mount "${StorageName}" "${NasFileSystemName}" "${MountPointExpected}"
        if [ $? -ne 0 ]; then
            Log "Task[${ParentTaskID}:${BackupID}], unable to mount target dataturbo filesystem: ${NasFileSystemName}, will exit!" 
            exit 1
        else
            Log "Task[${ParentTaskID}:${BackupID}], mount target dataturbo filesystem: ${NasFileSystemName} on ${MountPointExpected} successfully!"
            exit 0
        fi

    else
        Log "Task[${ParentTaskID}:${BackupID}], target nas filesystem is not provided, please check!"
        exit 1
    fi
fi

