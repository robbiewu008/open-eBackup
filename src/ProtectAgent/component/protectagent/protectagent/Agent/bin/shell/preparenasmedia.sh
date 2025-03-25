#!/bin/sh
set +x
#@function: mount nas share path

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
umask 0022
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
sysName=`uname -s`
#********************************define these for local script********************************
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for log
LOG_FILE_NAME="${LOG_PATH}/preparenasmedia.log"
#for GetValue

if [ "`ls -al /bin/sh | ${MYAWK} '{print $NF}'`" = "bash" ]; then
    EXPORT_ENV="if [ -f ~/.profile ]; then source ~/.profile; fi;"
else
    EXPORT_ENV=""
fi

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

serviceType=`GetValue "${PARAM_CONTENT}" serviceType`
StorageIP=`GetValue "${PARAM_CONTENT}" storageIp`
isLinkEncry=`GetValue "${PARAM_CONTENT}" linkEncryption`

test "$serviceType" = "${ERROR_PARAM_INVALID}"                           && ExitWithError "serviceType"
test "$StorageIP" = "${ERROR_PARAM_INVALID}"                             && ExitWithError "StorageIP"

storIPs=`echo ${StorageIP} | sed 's/;/ /g'`

# mark netwrok connection with nfs service
RetryTimes=3
ConnectFlag=0
LINK_ENCRY=
if [ ${isLinkEncry} -eq 1 ]; then
    LINK_ENCRY="sec=krb5p,vers=4.1,"
fi
IS_DPC=`cat ${AGENT_ROOT_PATH}/conf/agent_cfg.xml |grep '<is_dpc_compute_node' | awk -F '"' '{print $2}'`

Mount()
{
    ip=$1
    lun=$2
    localPath=$3
    # check net enable
    if [ $sysName = "Linux" ]; then
        su - rdadmin -s /bin/sh -l -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli testhost ${ip} 111 200">> $LOG_FILE_NAME 2>&1
    else
        su - rdadmin -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli testhost ${ip} 111 200">> $LOG_FILE_NAME 2>&1
    fi
    if [ $? -ne 0 ]; then
        Log "${ip} can't access."
        return 1
    fi

    [ ! -d ${localPath} ] && mkdir -p ${localPath}

    # check mount point
    currentFS=`df ${localPath} | sed '1d' | $MYAWK '{print $1}' | grep "${ip}:"`
    if [ $? -eq 0 -o ! -z "${currentFS}" ]; then
        echo ${currentFS} | grep -e ${ip} | grep -e ${lun}
        if [ $? -ne 0 ]; then
            Log "mounted ${currentFS}, need ${ip}:${lun}, try to umount."
        fi

        umount -f ${localPath} >> $LOG_FILE_NAME 2>&1
        if [ $? -ne 0 ]; then
            Log "umount ${localPath} failed"
        fi
    fi

    tempPath=${localPath%/*}
    while [ "${tempPath}" != "/tmp" ]; do
        strOwner=`ls -al "${tempPath}" | sed -n '2p' | $MYAWK '{print $3}'`
        if [ "${strOwner}" != "root" ]; then
            Log "Folder ${tempPath} owner group is ${strOwner}, not root."
            return 1
        fi
        tempPath=${tempPath%/*}
    done

    # mount
    if [ `CheckIsIpv6 ${ip}` -ne 0 ]; then
        if [ $sysName = "AIX" ]; then
            mount -o rw,bg,hard,nointr,rsize=262144,wsize=262144,timeo=600 -o noexec -o nosuid ${ip}":"${lun} ${localPath} >> $LOG_FILE_NAME 2>&1
        elif [ $sysName = "SunOS" ];then
            mount -F nfs -o vers=3,rw,bg,hard,nointr,rsize=262144,wsize=262144,timeo=600 ${ip}":"${lun} ${localPath} >> $LOG_FILE_NAME 2>&1
        else 
            if [ "$IS_DPC" = "true" ]
            then
                Log "Exec dpc mount ${lun} ${localPath}"
                mount -t dpc -o cnflush system":"${lun} ${localPath} >> $LOG_FILE_NAME 2>&1
            else
                mount -t nfs -o ${LINK_ENCRY}rw,bg,hard,nointr,rsize=262144,wsize=262144,tcp,actimeo=0,timeo=600,nolock,sync -o noexec -o nosuid ${ip}":"${lun} ${localPath} >> $LOG_FILE_NAME 2>&1
            fi
        fi           
    else
        if [ $sysName = "AIX" ]; then
            mount -o rw,bg,hard,nointr,rsize=262144,wsize=262144,timeo=600 -o noexec -o nosuid "["${ip}"]:"${lun} ${localPath} >> $LOG_FILE_NAME 2>&1
        elif [ $sysName = "SunOS" ];then
            mount -F nfs -o vers=3,rw,bg,hard,nointr,rsize=262144,wsize=262144,timeo=600 ${ip}":"${lun} ${localPath} >> $LOG_FILE_NAME 2>&1
        else 
            if [ "$IS_DPC" = "true" ]
            then
                Log "Exec dpc mount ${lun} ${localPath}"
                mount -t dpc -o cnflush "[system]:"${lun} ${localPath} >> $LOG_FILE_NAME 2>&1
            else
                mount -t nfs -o ${LINK_ENCRY}rw,bg,hard,nointr,rsize=262144,wsize=262144,actimeo=0,timeo=600,nolock,sync -o noexec -o nosuid "["${ip}"]:"${lun} ${localPath} >> $LOG_FILE_NAME 2>&1
            fi
        fi            
    fi
    # check mount status
    if [ $? -ne 0 ]; then
        Log "mount ${ip}:${lun} ${localPath} failed."
        return 1
    fi
    mount | grep ${localPath}
    if [ $? -ne 0 ]; then
        Log "mount ${ip}:${lun} ${localPath} failed."
        return 1
    fi
    return 0
}

MountData()
{
    storIPs=`echo $1 | sed 's/;/ /g'`
    if [ -z "${storIPs}" ]; then
        return
    fi
    for ip in ${storIPs}; do
        Mount "${ip}" "${DataSharePath}" "${DataMountPath}/${ip}"
        if [ $? -eq 0 ]; then
            chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}/${ip}
            if [ ! -d "${DataMountPath}/${ip}/data" ]; then
                if [ -d "${DataMountPath}/${ip}/additional" ]; then
                    chown -h -R root:root ${DataMountPath}/${ip}/additional
                    chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}/${ip}/additional/dbs
                    chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}/${ip}/additional/netadmin
                fi
            else
                if [ -d "${DataMountPath}/${ip}/data/additional" ]; then
                    chown -h -R root:root ${DataMountPath}/${ip}/data/additional
                    chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}/$ip/data/additional/dbs
                    chown -h -R ${ORA_DB_USER}:${ORA_DB_GROUP} ${DataMountPath}/${ip}/data/additional/netadmin
                fi                 
            fi
            mountDataPaths="${mountDataPaths}${DataMountPath}/${ip};"
            mountIPs="${mountIPs}${ip};"
        fi
    done
}

MountLog()
{
    storIPs=`echo $1 | sed 's/;/ /g'`
    if [ -z "${storIPs}" ]; then
        return
    fi
    for ip in ${storIPs}; do
        Mount "${ip}" "${LogSharePath}" "${LogMountPath}"
        if [ $? -eq 0 ]; then
            mountLogPaths=${LogMountPath}
            echo ${mountIPs} | grep ${ip}
            if [ $? -ne 0 ]; then
                mountIPs="${mountIPs}${ip};"
            fi
            break
        fi
    done
}

if [ "${serviceType}" = "database" ]; then
    GetOracleUser
    DataSharePath=`GetValue "${PARAM_CONTENT}" dataShareMountPath`
    LogSharePath=`GetValue "${PARAM_CONTENT}" logShareMountPath`
    DBNAME=`GetValue "${PARAM_CONTENT}" AppName`
    DBUUID=`GetValue "${PARAM_CONTENT}" DBUUID`
    DataOwnerIP=`GetValue "${PARAM_CONTENT}" dataOwnerIps`
    DataOtherIP=`GetValue "${PARAM_CONTENT}" dataOtherIps`
    LogOwnerIP=`GetValue "${PARAM_CONTENT}" logOwnerIps`
    LogOtherIP=`GetValue "${PARAM_CONTENT}" logOtherIps`

    test "$DataSharePath" = "${ERROR_PARAM_INVALID}"                            && ExitWithError "DataSharePath"
    test "$LogSharePath" = "${ERROR_PARAM_INVALID}"                             && ExitWithError "LogSharePath"
    test "$DBNAME" = "${ERROR_PARAM_INVALID}"                                   && ExitWithError "DBNAME"
    test "$DBUUID" = "${ERROR_PARAM_INVALID}"                                   && ExitWithError "DBUUID"
    test "$DataOwnerIP" = "${ERROR_PARAM_INVALID}"                              && ExitWithError "DataOwnerIP"
    test "$DataOtherIP" = "${ERROR_PARAM_INVALID}"                              && ExitWithError "DataOtherIP"
    test "$LogOwnerIP" = "${ERROR_PARAM_INVALID}"                               && ExitWithError "LogOwnerIP"
    test "$LogOtherIP" = "${ERROR_PARAM_INVALID}"                               && ExitWithError "LogOtherIP"

    Log "PID=${PID};DBNAME=${DBNAME};DBUUID=${DBUUID};\
    DataSharePath=${DataSharePath};DataOwnerIP=${DataOwnerIP};DataOtherIP=${DataOtherIP};\
    LogSharePath=${LogSharePath};LogOwnerIP=${LogOwnerIP};LogOtherIP=${LogOtherIP}."

    test -z "$DBNAME"           && ExitWithError "DBNAME"
    test -z "$DBUUID"           && ExitWithError "DBUUID"
    DataMountPath="/tmp/advbackup/data/"${DBUUID}
    LogMountPath="/tmp/advbackup/log/"${DBUUID}
    mountDataPaths=
    mountLogPaths=
    mountIPs=
    DeleteFile ${RESULT_FILE}

    if [ ! -z "${DataSharePath}" ]; then
        MountData "${DataOwnerIP}"
        if [ -z "${mountDataPaths}" ]; then
            MountData "${DataOtherIP}"
        fi
        test -z "$mountDataPaths" && ExitWithErrorCode "nas data path" ${ERROR_DISCONNECT_STORAGE_NETWORK}
    fi

    if [ ! -z "${LogSharePath}" ]; then
        MountLog "${LogOwnerIP}"
        if [ -z "${mountLogPaths}" ]; then
            MountLog "${LogOtherIP}"
        fi
        test -z "$mountLogPaths" && ExitWithErrorCode "nas log path" ${ERROR_DISCONNECT_STORAGE_NETWORK}
    fi

    echo ${mountDataPaths} > "${RESULT_FILE}"
    echo ${mountLogPaths} >> "${RESULT_FILE}"
    echo ${mountIPs} >> "${RESULT_FILE}"
    Log "Mount oracle nfs success."
elif [ "${serviceType}" = "vmware" ]; then
    # vmware feature usage
    NasFileSystemName=`GetValue "${PARAM_CONTENT}" nasFileSystemName`
    # parent task id
    BackupID=`GetValue "${PARAM_CONTENT}" backupID`
    ParentTaskID=`GetValue "${PARAM_CONTENT}" parentTaskID`

    test "$NasFileSystemName" = "${ERROR_PARAM_INVALID}"                       && ExitWithError "NasFileSystemName"
    test "$BackupID" = "${ERROR_PARAM_INVALID}"                                && ExitWithError "BackupID"
    test "$ParentTaskID" = "${ERROR_PARAM_INVALID}"                            && ExitWithError "ParentTaskID"

    Log "PID=${PID};NasFileSystemName=${NasFileSystemName};StorageIP=${StorageIP};ParentTaskID=${ParentTaskID};BackupID=${BackupID}."
    test -z "$NasFileSystemName"       && ExitWithError "NasFileSystemName"
    test -z "$StorageIP"               && ExitWithError "StorageIP"
    test -z "$BackupID"                && ExitWithError "BackupID"

    # generate nas filesystem mnt expected
    MountPointPrefix="/opt/advbackup/vmware/data"
    MountPointExpected="${MountPointPrefix}/${BackupID}"
    if [ ! -z "${NasFileSystemName}" ]; then
        AvailableStorageIp=
        # create mnt path
        if [ ! -d ${MountPointExpected} ]; then
            mkdir -p ${MountPointExpected} >> $LOG_FILE_NAME 2>&1
            chmod 755 "/opt/advbackup"
            chmod 700 -R ${MountPointExpected}
        fi
        Log "Task[${ParentTaskID}:${BackupID}], mountpoint expected: ${MountPointExpected}"

        # check connection of the dest ip
        while [ "${RetryTimes}" -gt 0 ]; do
            # find a available nfs service ip
            for ip in ${storIPs}; do
                su - rdadmin -s /bin/sh -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli testhost ${ip} 111 200">> $LOG_FILE_NAME 2>&1
                if [ $? -ne 0 ]; then
                    Log "Task[${ParentTaskID}:${BackupID}], unable to connect to remote nas service with ip ${ip}, will try again."
                    # due to current nfs service network unreachable, will try the next one
                    continue
                else
                    ConnectFlag=1
                    AvailableStorageIp="${ip}"
                    break
                fi
            done

            if [ "${ConnectFlag}" -eq 1 ]; then
                break
            else
                RetryTimes=`expr ${RetryTimes} - 1`
                sleep 10
                continue
            fi
        done

        if [ "${ConnectFlag}" -ne 1 ]; then
            Log "Task[${ParentTaskID}], all nfs service network are unreachable, will exit."
            exit 1
        fi
    
        # mount nfs storage using the accessable ip
        # generate nas filesystem
        TargetNasFileSystem="${AvailableStorageIp}:/${NasFileSystemName}"
        Log "Task[${ParentTaskID}:${BackupID}], target nas filesystem: ${TargetNasFileSystem}"
        # check whether the expected nas alreadly been mounted on expected mnt
        mount | grep "${TargetNasFileSystem}" | $MYAWK '{print $3}' | grep "${MountPointExpected}"
        if [ $? -eq 0 ]; then
            # skip once nas filesystem has alreadly been mounted -- both mnt and nas filesystem are the same

            Log "Task[${ParentTaskID}:${BackupID}], target nas filesystem: ${TargetNasFileSystem} has alreadly been mounted on: ${MountPointExpected}"
            exit 0
        fi
        MOUNT_VERS="vers=3,"
        if [ ! -z "${LINK_ENCRY}" ]; then
            MOUNT_VERS=${LINK_ENCRY}
        fi
        # mount nfs
        if [ `CheckIsIpv6 ${AvailableStorageIp}` -ne 0 ]; then
            mount -v -t nfs -o ${MOUNT_VERS}rw,soft,noatime,nodiratime,rsize=262144,wsize=262144,tcp,actimeo=0,timeo=60,retry=5,nolock,sync -o noexec -o nosuid ${AvailableStorageIp}":/"${NasFileSystemName} ${MountPointExpected} >> $LOG_FILE_NAME 2>&1
        else
            mount -v -t nfs -o ${MOUNT_VERS}rw,soft,noatime,nodiratime,rsize=262144,wsize=262144,actimeo=0,timeo=60,retry=5,nolock,sync -o noexec -o nosuid "["${AvailableStorageIp}"]:/"${NasFileSystemName} ${MountPointExpected} >> $LOG_FILE_NAME 2>&1
        fi

        if [ $? -ne 0 ]; then
            Log "Task[${ParentTaskID}:${BackupID}], unable to mount target nas filesystem: ${TargetNasFileSystem}, will exit!" 
            exit 1
        else
            Log "Task[${ParentTaskID}:${BackupID}], mount target nas filesystem: ${TargetNasFileSystem} on ${MountPointExpected} successfully!"
            exit 0
        fi
    else
        Log "Task[${ParentTaskID}:${BackupID}], target nas filesystem is not provided, please check!"
        exit 1
    fi
elif [ "${serviceType}" = "archivestream" ];then
    # archivestream feature usage
    NasFileSystemName=`GetValue "${PARAM_CONTENT}" nasFileSystemName`
    LocalPath=`GetValue "${PARAM_CONTENT}" LocalPath`

    Log "PID=${PID};NasFileSystemName=${NasFileSystemName};StorageIP=${StorageIP};LocalPath=${LocalPath}."
    test -z "$NasFileSystemName"       && ExitWithError "NasFileSystemName"
    test -z "$StorageIP"               && ExitWithError "StorageIP"
    test -z "$LocalPath"               && ExitWithError "LocalPath"

    if [ ! -z "${NasFileSystemName}" ]; then
        ip=${StorageIP}
        # check connection of the dest ip
        su - rdadmin -s /bin/sh -c "${EXPORT_ENV}${AGENT_ROOT_PATH}/bin/agentcli testhost ${ip} 111 3000" >> $LOG_FILE_NAME 2>&1
        if [ $? -ne 0 ]; then
            Log "unable to connect to remote nas service with ip ${ip}"
            exit 1
        fi

        # generate path
        RemotePath="${ip}:/${NasFileSystemName}"
        GrepRemotePath="${ip}:/${NasFileSystemName}"
        if [ `CheckIsIpv6 ${ip}` -eq 0 ]; then
            RemotePath="["${ip}"]:/"${NasFileSystemName}
            GrepRemotePath="\["${ip}"\]:/"${NasFileSystemName}
        fi
        Log "RemotePath: ${RemotePath}; GrepRemotePath:${GrepRemotePath}"

        # check whether already mounted
        mount | grep "${GrepRemotePath} on" | $MYAWK '{print $3}' | grep "${LocalPath}"
        if [ $? -eq 0 ]; then
            Log "target nas filesystem: ${RemotePath} has alreadly been mounted on: ${LocalPath}"
            exit 0
        fi

        # create dir and change owner
        if [ ! -d ${LocalPath} ]; then
            mkdir -p ${LocalPath} >> $LOG_FILE_NAME 2>&1
            FatherPath=`dirname ${LocalPath}`
            chmod 555 ${FatherPath}
            chmod 555 ${LocalPath}
            Log "mkdir,FatherPath=${FatherPath}."
        fi

        # do mount
        MountOptions="-t nfs -o ${LINK_ENCRY}rw,hard,noatime,nodiratime,rsize=262144,wsize=262144,tcp,actimeo=0,timeo=60,retry=5,nolock,sync -o noexec -o nosuid"
        if [ $sysName = "AIX" ]; then
            MountOptions="-o rw,bg,hard,nointr,rsize=262144,wsize=262144,timeo=600 -o noexec -o nosuid"
        elif [ $sysName = "SunOS" ];then
            MountOptions="-o vers=3,rw,bg,hard,nointr,rsize=262144,wsize=262144,timeo=600"
        fi

        Log "mount command: mount ${MountOptions} ${RemotePath} ${LocalPath}"
        Timeout mount ${MountOptions} ${RemotePath} ${LocalPath} >> $LOG_FILE_NAME 2>&1
        if [ $? -ne 0 ]; then
            Log "unable to mount target nas filesystem: ${RemotePath}!remove ${LocalPath}!"
            rmdir ${LocalPath} >> $LOG_FILE_NAME 2>&1
            rmdir ${FatherPath} >> $LOG_FILE_NAME 2>&1
            exit 1
        else
            Log "mount target nas filesystem: ${RemotePath} on ${LocalPath} successfully!"
        fi

        # while a new filesystem created, its defult owner is root, we must change it
        chown rdadmin:rdadmin ${LocalPath}
    else
        Log "target nas filesystem is not provided, please check!"
        exit 1
    fi
else
    Log "Invalid service type=${serviceType}."
    exit 1
fi
exit 0

