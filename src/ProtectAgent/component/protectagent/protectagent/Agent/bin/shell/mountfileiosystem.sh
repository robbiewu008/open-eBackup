#!/bin/sh
set +x
#@function: mount nas share path

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

#for log
LOG_FILE_NAME="${LOG_PATH}/mountfileiosystem.log"
TMP_DIR="/mnt"


BLOCK_LIST="^/$\|^/tmp$\|^/mnt/databackup$\|^/bin$\|^/bin/.*\|^/boot$\|^/boot/.*\|^/dev$\|^/dev/.*\|^/etc$\|^/etc/.*\|^/lib$\|^/lib/.*\|^/lost+found$\|^/lost+found/.*\|^/media$\|^/media/.*\|^/mnt$\|^/opt$\|^/proc$\|^/proc/.*\|^/root$\|^/root/.*\|^/sbin$\|^/sbin/.*\|^/selinux$\|^/selinux/.*\|^/srv$\|^/srv/.*\|^/sys$\|^/sys/.*\|^/usr$\|^/usr/.*\|^/var$\|^/var/.*\|^/run$\|^/run/.*"


CheckFileSystem()
{
    LvName=$1
    retryTime=1
    while [ $retryTime -le 3 ]; do
        mount | grep $LvName | grep ${HostMountPath}
        if [ $? -eq 0 ]; then
            Log "Check mount ${LvName} mountpath ${HostMountPath} success."
            return 0
        fi
        Log "Mount $LvName ${HostMountPath} check failed retry $retryTime time"
        if [ "${SYS_NAME}" = "AIX" ]; then
            Timeout mount ${HostMountPath} >> $LOG_FILE_NAME 2>&1
        else
            Log "Not support os"
            return 0
        fi
        retryTime=`expr $retryTime + 1`
        if [ $retryTime -ge 4 ]; then
            Log "Check filesystem time out."
            return 1
        fi
        sleep 5
    done
}

ChangeUser()
{
    if [ $# -ne 2 ]; then
        return 0
    fi
    path=$1
    uid=$2
    if [ -n "${uid}" ]; then
        chown ${uid} ${path}
    fi
    return 0
}

CheckMountPath()
{
    realMountPath=""
    if [ "${SYS_NAME}" = "AIX" ]; then
        cd $1
        realMountPath=`pwd`
        cd -
    else
        mountPath=$1
        realMountPath=`realpath ${mountPath}`
    fi
    expr match ${realMountPath} ${BLOCK_LIST} > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "The mount path is in the blocklist, path is ${mountPath}."
        return 1
    fi
}

RefreshDev()
{
    DevName=$1
    if [ "${SYS_NAME}" != "AIX" ]; then
        return 0
    fi
    cfgmgr -v -l ${DevName} > /dev/null 2>&1
    if [ $? -ne 0 ]; then
      Log "Scan ${DevName} failed."
      return 1
    fi
    return 0
}

ListAdapter()
{
    if [ "${SYS_NAME}" != "AIX" ]; then
        exit 0
    fi
    AdapterType=$1
    Adapters=`lsdev -Cc adapter | grep ${AdapterType} | ${MYAWK} '{print $1}'`
    if [ -z "${Adapters}" ]; then
        Log "Not find ${AdapterType} Adapter."
        exit 1
    fi
    echo ${Adapters}
}

QueryAdapterWwpn()
{
    adpterType=$1
    adpterWwpn=$2
    if [ "${adpterType}" = "Fibre" ]; then
        Adapters=`ListAdapter ${adapterType}`
        for fc in ${Adapters}; do
            tmpWwpn=`lscfg -pvl ${fc}|grep "Network Address"|grep -i ${adpterWwpn}`
            if [ $? -eq 0 ]; then
                echo "${fc}"
                return 0
            fi
        done
        Log "Not much agent wwpn with this lun."
        return 1
    fi
    return 0
}

ScanAdapter()
{
    adpterType=$1
    adpterWwpn=$2
    sanclientIP=$3
    sanclientPort=$4
    if [ "${adpterType}" = "Fibre" ]; then
        fc=`QueryAdapterWwpn ${adapterType} ${adpterWwpn}`
        if [ $? -ne 0 ]; then
            Log "Not find dev with adpter type ${adpterType} and ${adpterWwpn}."
            return 1
        fi
        RefreshDev ${fc}
        if [ $? -ne 0 ]; then
            Log "Scan ${fc} failed, ignore it and continue."
        fi
    else
        iscsi=`lsdev | grep iscsi | ${MYAWK} '{print $1}'`
        if [ $? -ne 0 ]; then
            Log "Not find dev with adpter type ${adpterType}."
            return 1
        fi
        Log "Scan ${iscsi}."
        SetOneWayAuthentication ${sanclientIP} ${wwpn} ${unidirectionalAuthPwd} ${sanclientPort} ${adpterWwpn}
        RefreshDev ${iscsi}
        if [ $? -ne 0 ]; then
            Log "Scan ${iscsi} failed, ignore it and continue."
        fi
    fi
}

SetOneWayAuthentication()
{
	chmod 600 /etc/iscsi/targets
    SanclientIp=$1
    SanclientIqn=$2
    SanclientToken=$3
    sanclientPort=$4
    agentIqn=$5
    isSetAuth=`grep ${SanclientToken} /etc/iscsi/targets`
    iqnName=`lsattr -El iscsi0 | grep initiator_name | ${MYAWK} '{print $2}'`
    if [ "${iqnName}" != "${agentIqn}" ]; then
        return 0
    fi
    if [ "${isSetAuth}" = "" ]; then
        Log "Start configuring chap authentication."
	    grep -v ${SanclientIqn} /etc/iscsi/targets > /etc/iscsi/targets_tmp
        mv /etc/iscsi/targets_tmp /etc/iscsi/targets
	    echo "${SanclientIp} ${sanclientPort} ${SanclientIqn} \"${SanclientToken}\"" | tr -d '\r' >> /etc/iscsi/targets
    fi
}

GetDiskByLunAndWwpn()
{
    sanclientWwpn=$1
    lunId=`printf '%x' $2`
    Log "Begin to search disk by wwpn: ${sanclientWwpn} and lunid 0x: ${lunId}."
    disks=`lsdev -Cc disk | grep Available | ${MYAWK} '{print $1}'`
    for disk in ${disks}; do
        ww_name=`lsattr -El ${disk} -F value -a ww_name | sed -e s/0x//`
        if [ "${ww_name}" != "${sanclientWwpn}" ]; then
           continue
        fi
        lun_id=`lsattr -El ${disk} -F value -a lun_id | sed -e s/0x// | sed -e s/${lunId}/Z/ |sed -e s/0//g`
        if [ "${lun_id}" == 'Z' ]; then
            echo "${disk}"
            Log "Find disk is ${disk}, lunid is ${lunId}."
            return 0
        fi
    done
    Log "Not find target fileio with lun_id ${lunId}."
    return 1
}

GetDiskByLunAndIqn()
{
    sanclientIqn=$1
    lunId=`printf '%x' $2`
    Log "Begin to search disk by iqn and lunid 0x: ${lunId}."
    disks=`lsdev -Cc disk | ${MYAWK} '{print $1}'`
    for disk in ${disks}; do
        target_name=`lsattr -El ${disk} -F value -a target_name`
        if [ "${target_name}" != "${sanclientIqn}" ]; then
           continue
        fi
        lun_id=`lsattr -El ${disk} -F value -a lun_id | sed -e s/0x// | sed -e s/${lunId}/Z/ |sed -e s/0//g`
        if [ "${lun_id}" == 'Z' ]; then
            echo "${disk}"
            Log "Find disk is ${disk}, lunid is ${lunId}."
            return 0
        fi
    done
    Log "Not find target fileio with lun_id ${lunId}."
    return 1
}

MountLvm()
{
    vgName=$1
    fileioName=$2
    jobType=$3
    logPath=`lsvg -l "${vgName}" | grep  -w "jfs2log" | ${MYAWK} '{print $1}'`
    lvmPath=`lsvg -l "${vgName}" | grep  -w "jfs2" | ${MYAWK} '{print $1}'`
    if [ -z "${logPath}" -o -z "${lvmPath}" ]; then
        Log "Vg: ${vgName} not exsist log lvm ${logPath} or lvm ${lvmPath}."
        return ${ERROR_CREATE_LV}
    fi
    LvmName=${lvmPath}
    tmpHostMountPath=${HostMountPath}
    # 恢复任务需要更改挂载点
    if [ ${jobType} -eq 2 ]; then
        repoID=`echo ${fileioName} | $MYAWK -F '_' '{print $1}'`
        mkdir ${HostMountPath}/${repoID}
        tmpHostMountPath="${HostMountPath}/${repoID}"
    fi
    # 检查是否挂载
    mount | grep "${lvmPath}" >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        Log "Begin to mount manual with ${logPath} and ${lvmPath}"
        mount -o log="/dev/${logPath}" "/dev/${lvmPath}" "${tmpHostMountPath}" >> ${LOG_FILE_NAME} 2>&1
        return $?
    fi
    Log "Lv:${lvmPath} is mounted, check mount point."
    mount | grep "${lvmPath}" | grep "${tmpHostMountPath}" >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "Lv:${lvmPath} is mounted on ${tmpHostMountPath} success."
        return 0
    fi
    Log "Lv:${lvmPath} not mounted on this path, change mount point."
    srcMountPath=`mount | grep "${lvmPath}" | ${MYAWK} '{print $2}'`
    chfs -m "${tmpHostMountPath}" "${srcMountPath}"
    umount ${srcMountPath}
    mount -o log="/dev/${logPath}" "/dev/${lvmPath}" "${tmpHostMountPath}" >> ${LOG_FILE_NAME} 2>&1
    return $?
}

ImportVg()
{
    diskName=$1
    vgName=$2
    lvName=$3
    fileSize=$4
    fileioName=$5
    jobType=$6
    # 检查importvg信息
    tmpVgName=`lspv | grep -w "${diskName}"| ${MYAWK} '{print $3}'`
    if [ -n "${tmpVgName}" ] && [ "${tmpVgName}" != "None" ]; then
        vgName=${tmpVgName}
        Log "Vg:${vgName} is imported."
    else
        importvg -y ${vgName} ${diskName} >> ${LOG_FILE_NAME} 2>&1
        if [ $? -ne 0 ]; then
            Log "Import vg ${vgName} failed.May not find vg on this pv."
            mkvg -Sy ${vgName} ${diskName} >> ${LOG_FILE_NAME} 2>&1
            if [ $? -ne 0 ]; then
                Log "Make vg: ${vgName} failed."
                Clear ${diskName} ${vgName}
                return ${ERROR_CREATE_VG}
            fi
        fi
    fi
    # 检查是否激活

    isActive=`lspv | grep -w ${diskName} | ${MYAWK} '{print $NF}'`
    if [ "${isActive}" != "active" ]; then
        varyonvg ${vgName} >> ${LOG_FILE_NAME} 2>&1
        if [ $? -ne 0 ]; then
            Log "Varyonvg  ${vgName} failed."
            Clear ${diskName} ${vgName}
            return ${ERROR_VARYONVG}
        fi
    fi
    lsvg -l ${vgName}
    if [ $? -ne 0 ]; then
        Log "Query vg info failed, may lose connect, check it."
        Clear ${diskName}
        return ${ERROR_CREATE_VG}
    fi

    #检查是vg里是否有逻辑卷，可能vg未创建逻辑卷
    lsvg -l ${vgName} | grep -v "${vgName}:" | grep -v "LV NAME"
    if [ $? -ne 0 ]; then  #未创建lvm
        Log "Vg:${vgName} not have lv , begin to create. "
        CreateLvm ${vgName} ${lvName} ${fileSize}
        if [ $? -ne 0 ]; then
            Log "Create lv ${lvName} failed with import vg ${vgName}."
            Clear ${diskName} ${vgName}
            return ${ERROR_CREATE_LV}
        fi
        return 0
    fi
    MountLvm ${vgName} ${fileioName} ${jobType}
    if [ $? -ne 0 ]; then
        Log "Mount ${lvmPath} failed."
        Clear ${diskName} ${vgName}
        return ${ERROR_MOUNTPATH}
    fi
    # 确保激活，忽略报错
    varyonvg ${vgName} >> /dev/null 2>&1
    CheckFileSystem ${lvmPath}
    Log "Create vg with importvg sucess."
    return 0
}

CreateVg()
{
    wwpn=$1
    lunId=$2
    vgName=$3
    lvName=$4
    fileSize=$5
    adapterType=$6
    agentWwpn=$7
    fileioName=$8
    jobType=$9
    sanclientIP=${10}
    unidirectionalAuthPwd=${11}
    sanclientPort=${12}
    Log "Begin to create vg with param. ${lunId} ${vgName} ${lvName} ${fileSize}"
    if [ "${adapterType}" = "ISCSI" ]; then
        diskName=`GetDiskByLunAndIqn ${wwpn} ${lunId}`
    else
        diskName=`GetDiskByLunAndWwpn ${wwpn} ${lunId}`
    fi
    if [ $? -ne 0 ]; then
        Log "Scan disk failed, not find any disk info. try to Scan ${adapterType} adapter "
        ScanAdapter ${adapterType} ${agentWwpn} ${sanclientIP} ${sanclientPort}
        if [ $? -ne 0 ]; then
            Log "No adapter on this node, no need create vg."
            return ${ERROR_SCAN_ADAPTER}
        fi
        if [ "${adapterType}" = "ISCSI" ]; then
            diskName=`GetDiskByLunAndIqn ${wwpn} ${lunId}`
        else
            diskName=`GetDiskByLunAndWwpn ${wwpn} ${lunId}`
        fi
    fi
    if [ -z "${diskName}" ]; then
        Log "Scan disk failed, not find any fc disk."
        return ${ERROR_SCAN_DISK}
    fi
    pvId=`lsattr -El ${diskName} -F value -a pvid`
    Log "Find new disk, pvId is ${pvId}."
     # pv有信息，可能以前挂载过，尝试导入vg
    if [ -n "${pvId}" ] && [ "${pvId}" != "none" ]; then
       ImportVg ${diskName} ${vgName} ${lvName} ${fileSize} ${fileioName} ${jobType}
       return $?
    fi

    # 新接入文件系统 进行新创建
    # 创建pv
    chdev -l ${diskName} -a pv=yes  >> ${LOG_FILE_NAME} 2>&1
    if [ $? -ne 0 ]; then
        Log "Create pv failed, clear disk."
        Clear ${diskName}
        return ${ERROR_CREATE_PV}
    fi
    # 创建vg
    mkvg -Sy ${vgName} ${diskName} >> ${LOG_FILE_NAME} 2>&1
    if [ $? -ne 0 ]; then
        Log "Make vg: ${vgName} failed."
        Clear ${diskName}
        return ${ERROR_CREATE_VG}
    fi
    # 创建lvm
    CreateLvm ${vgName} ${lvName} ${fileSize}
    if [ $? -ne 0 ]; then
        Log "Make lvm :${lvName} failed."
        Clear ${diskName} ${vgName}
        return ${ERROR_CREATE_LV}
    fi
    # 检查挂载情况
    CheckFileSystem ${lvName}
    return 0

}

CreateLvm()
{
    vgName=$1
    lvName=$2
    # 逻辑卷大小创建为传入的90%
    size=`expr $3 \* 9 / 10`
    Log "Begin to create lvm with vgname: ${vgName}, ${lvName} size:${size}"
    mklv -y "${lvName}" -t jfs2 "${vgName}" "${size}G" >> ${LOG_FILE_NAME} 2>&1
    if [ $? -ne 0 ]; then
        Log "Create lv ${lvName} failed."
        return ${ERROR_CREATE_LV}
    fi
    varyonvg ${vgName} >> ${LOG_FILE_NAME} 2>&1
    Log "Begin to crfs mount ${lvName} ${HostMountPath}."
    crfs -v jfs2  -m ${HostMountPath} -d ${lvName}
    if [ $? -ne 0 ]; then
        Log "Mount ${lvName} ${HostMountPath} failed with crfs."
    fi
    Log "Lv:${lvName} created and crfs success."
    MountLvm ${vgName}
    return $?
}

MountLunInfo()
{
    Log "Begin to mount lun info.$2"
    LunInfo=$1
    protocolType=$2
    ret=${ERROR_MOUNTPATH}
    resourceId=${LunInfo%%:*}
    if [ ${protocolType} = "fc" ]; then
        adapterType="Fibre"
    else
        adapterType="ISCSI"
    fi
    # 数组实现方式适用于AIX环境
    set -A lunId_success
    set -A all_lunId
    lunNum=0
    for info in `echo ${LunInfo#*:} | $MYAWK -F '//' '{for(i=1;i<NF;i++){print $i}}'`; do
        wwpntmp=${info%/*}
        protocolTmp=`echo ${wwpntmp} | $MYAWK -F ',' '{print $1}'`
        protocolId=`echo ${protocolTmp} | $MYAWK -F '_' '{print $1}'`
        sanclientIP=`echo ${protocolTmp} | $MYAWK -F '_' '{print $2}'`
        unidirectionalAuthPwd=`echo ${wwpntmp} | $MYAWK -F ',' '{print $2}'`
        lunTmp=${info#*/}
        lunId=`echo ${lunTmp} | $MYAWK -F ',' '{print $1}'`
        tmp1=`echo ${all_lunId[@]} | grep -w ${lunId}`
        if [ "${tmp1}" == "" ]; then
            set -A all_lunId `echo ${all_lunId[*]} ${lunId}`
            lunNum=`expr ${lunNum} + 1`
        fi
        tmp=`echo ${lunId_success[@]} | grep -w ${lunId}`
        if [ "${tmp}" != "" ]; then
            Log "lunid ${lunId} has been operated, skip it."
            continue
        fi
        repoType=`echo ${lunTmp} | $MYAWK -F ',' '{print $2}'`
        fileSystemSize=`echo ${lunTmp} | $MYAWK -F ',' '{print $3}'`
        agentWwpn=`echo ${lunTmp} | $MYAWK -F ',' '{print $4}'`
        fileioName=`echo ${lunTmp} | $MYAWK -F ',' '{print $5}'`
        jobType=`echo ${lunTmp} | $MYAWK -F ',' '{print $6}'`
        sanclientPort=`echo ${lunTmp} | $MYAWK -F ',' '{print $7}'`
        VgName="vg${resourceId}${lunId}"
        LvmName="${VgName}lvm"
        Log "Begin to mount with param vgname:${VgName} lvmname:${LvmName} Size:${fileSystemSize}"
        CreateVg ${protocolId} ${lunId} ${VgName} ${LvmName} ${fileSystemSize} ${adapterType} ${agentWwpn} ${fileioName} ${jobType} ${sanclientIP} ${unidirectionalAuthPwd} ${sanclientPort} >> ${LOG_FILE_NAME} 2>&1
        if [ $? -eq 0 ]; then
            Log "Create lv sucess: ${LvmName}."
            set -A lunId_success `echo ${lunId_success[*]} ${lunId}`
            continue
        fi
        Log "Create vg failed, lun id is ${lunId}, try next."
    done
    successLunNum=`echo ${#lunId_success[*]}`
    if [ "${successLunNum}" -eq "${lunNum}" ]; then
        return 0
    fi
    Log "Create vg failed, with all lun, exit failed."
    return ${ERROR_CREATE_VG}
}

Clear()
{
    disk=$1
    vg=$2
    Log "Begin to clear disk ${disk}."
    lspv | grep -w "${disk}"
    if [ $? -ne 0 ]; then
        Log "Not find disk ${disk}, no need release."
        return 0
    fi
    if [ -n "${vg}" ]; then
        Log "Begin to exportvg ${vg}."
        varyoffvg ${vg} >>  ${LOG_FILE_NAME} 2>&1
        exportvg ${vg} >> ${LOG_FILE_NAME} 2>&1
    fi
    rmdev -dl ${disk}
    if [ $? -ne 0 ]; then
        Log "Remove ${disk} failed"
        return 1
    fi
    Log "Clear ${disk} complete."
    return 0
}

MountLunInfoRetry()
{
    LunInfo="$1"
    ProtocolType="$2"
    retryTime=1
    while [ $retryTime -le 5 ]; do
        MountLunInfo ${LunInfo} ${ProtocolType}
        if [ $? -eq 0 ]; then
            return 0
        fi

        Log "Create mount lvm failed, retry times: $retryTime"
        retryTime=`expr $retryTime + 1`
        if [ $retryTime -ge 6 ]; then
            Log "Create mount lvm time out."
            return 1
        fi
        sleep 5
    done
    return 1
}

Log "********************************Start to execute the fileio mount script********************************"

# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"


#for GetValue
HostMountPath=`GetValue "${PARAM_CONTENT}" mountPath`
ProtocolType=`GetValue "${PARAM_CONTENT}" protocolType`
RepositoryType=`GetValue "${PARAM_CONTENT}" repositoryType`
UserID=`GetValue "${PARAM_CONTENT}" uid`
GroupID=`GetValue "${PARAM_CONTENT}" gid`
Mode=`GetValue "${PARAM_CONTENT}" mode`
SubPath=`GetValue "${PARAM_CONTENT}" subPath`
LunInfo=`GetValue "${PARAM_CONTENT}" LunInfo`
RecordFile=`GetValue "${PARAM_CONTENT}" recordFile`

test "$HostMountPath" = "${ERROR_PARAM_INVALID}"            && ExitWithError "HostMountPath"
test "$ProtocolType" = "${ERROR_PARAM_INVALID}"            && ExitWithError "ProtocolType"
test "$RepositoryType" = "${ERROR_PARAM_INVALID}"           && ExitWithError "RepositoryType"

test -z "$HostMountPath"           && ExitWithError "HostMountPath"
test -z "$ProtocolType"           && ExitWithError "ProtocolType"
test -z "$RepositoryType"          && ExitWithError "RepositoryType"


Log "PID=${PID};HostMountPath=${HostMountPath};RepositoryType=${RepositoryType};UserID=${UserID};GroupID=${GroupID}; SubPath=${SubPath};RecordFile=${RecordFile}."

mount | grep ${HostMountPath}
if [ $? -eq 0 ];then
    Log "${HostMountPath} is mounted, no need mount again."
    if [ -n "${RecordFile}" ]; then
        ls "${RecordFile}" > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            Log "${HostMountPath} recorded in meta repository."
            exit 0
        fi
        touch "${RecordFile}" >> /dev/null 2>&1
    fi
    exit 0
fi

if [ ! -d "${HostMountPath}" ]; then
    mkdir -p "${HostMountPath}" >> $LOG_FILE_NAME 2>&1
    if [ $? -ne 0 ]; then
        Log "Ceate mount point failed."
        exit ${ERROR_MOUNTPATH}
    fi
fi

subHostMountPath=${HostMountPath#*/}
subHostMountPath=${subHostMountPath#*/}
dirList=`echo ${subHostMountPath} | sed 's/\// /g'`
cd ${TMP_DIR}
for dirName in ${dirList}; do
    chmod 555 ${dirName}
    cd ${dirName}
done
CheckMountPath ${HostMountPath} >> ${LOG_FILE_NAME} 2>&1
if [ -n "${RecordFile}" ]; then
    ls "${RecordFile}" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        Log "Fileio mounted on other node, can not mount on this node."
        exit 0
    fi
    touch "${RecordFile}" >> /dev/null 2>&1
fi

# Add retry to MountLunInfo method
MountLunInfoRetry ${LunInfo} ${ProtocolType}
ret=$?
if [ $ret -ne 0 ]; then
    Log "Create mount lvm failed."
    Timeout rm -f "${RecordFile}" >> /dev/null 2>&1
    exit $ret
fi

Log "Create lvm ${LvName} complete. "




appGroupName=${AGENT_GROUP}


if [ -n "${GroupID}" ]; then
    appGroupName=${GroupID}
fi

mode=755
appMode=${mode}
if [ -n "${Mode}" ]; then
    appMode=${Mode}
fi

# 日志仓根目录固定给755 root:root, 任务ID目录根据应用返回权限设置
if [ ${RepositoryType} = "log" ]; then
    chmod 755 ${HostMountPath}
    chown root:root ${HostMountPath}
    if [ ! -f ${HostMountPath}/.agentlastlogbackup.meta ]; then
        touch ${HostMountPath}/.agentlastlogbackup.meta
        chmod 640 ${HostMountPath}/.agentlastlogbackup.meta
        chown ${AGENT_USER}:${AGENT_GROUP} ${HostMountPath}/.agentlastlogbackup.meta
    fi
    if [ ! -f ${HostMountPath}/.dmelastlogbackup.meta ]; then
        touch ${HostMountPath}/.dmelastlogbackup.meta
        chmod 640 ${HostMountPath}/.dmelastlogbackup.meta
        chown ${AGENT_USER}:${AGENT_GROUP} ${HostMountPath}/.dmelastlogbackup.meta
    fi
    subPathList=`echo ${SubPath} | sed 's/:/ /g'`
    for path in ${subPathList}; do
        tmpPath="${HostMountPath}/${path}"
        [ ! -d "${tmpPath}" ] && mkdir -p "${tmpPath}"
        if [ "${RunAccount}" = "rdadmin" ]; then
            chmod ${appMode} ${tmpPath}
            chown ${AGENT_USER}:${appGroupName} ${tmpPath}
        fi
        if [ -n "${GroupID}" ]; then
            chgrp ${appGroupName} ${tmpPath}
        fi
        if [ -n "${Mode}" ]; then
            chmod ${appMode} ${tmpPath}
        fi
        ChangeUser ${tmpPath} ${UserID}
    done
fi

if [ ${RepositoryType} = "cache" ]; then
    chmod 755 ${HostMountPath}
    chown root:root ${HostMountPath}
    if [ "${RunAccount}" = "rdadmin" ]; then
        chmod ${appMode} ${HostMountPath}
        chown ${AGENT_USER}:${appGroupName} ${HostMountPath}
    fi
    subPathList=`echo ${SubPath} | sed 's/:/ /g'`
    for path in ${subPathList}; do
        tmpPath="${HostMountPath}/${path}"
        [ ! -d "${tmpPath}" ] && mkdir -p "${tmpPath}"
        if [ "${RunAccount}" = "rdadmin" ]; then
            chmod ${appMode} ${tmpPath}
            chown ${AGENT_USER}:${appGroupName} ${tmpPath}
        fi
        if [ -n "${GroupID}" ]; then
            chgrp ${appGroupName} ${tmpPath}
        fi
        if [ -n "${Mode}" ]; then
            chmod ${appMode} ${tmpPath}
        fi
        ChangeUser ${tmpPath} ${UserID}
    done
fi

if [ "${RunAccount}" = "rdadmin" ]; then
    if [ ${RepositoryType} = "meta" ] || [ ${RepositoryType} = "data" ] || [ ${RepositoryType} = "index" ]; then
        chmod ${appMode} ${HostMountPath}
        chown ${AGENT_USER}:${appGroupName} ${HostMountPath}
        ChangeUser ${HostMountPath} ${UserID}
    fi
else
    if [ ${RepositoryType} = "meta" ] || [ ${RepositoryType} = "data" ] || [ ${RepositoryType} = "index" ]; then
        if [ -n "${GroupID}" ]; then
            chgrp ${appGroupName} ${HostMountPath}
        fi
        if [ -n "${Mode}" ]; then
            chmod ${appMode} ${HostMountPath}
        fi
        ChangeUser ${HostMountPath} ${UserID}
    fi
fi

Log "Mount ${LvPath} ${HostMountPath} success."
exit 0
