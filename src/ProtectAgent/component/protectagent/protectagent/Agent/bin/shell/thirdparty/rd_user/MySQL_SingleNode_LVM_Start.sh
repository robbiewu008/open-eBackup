#!/bin/sh
set +x

. "$2/bin/agent_thirdpartyfunc.sh"

function MountLVM()
{   
    local num=0
    LUN_ID_TEMP_ARR=(${LunIdMountPath//###/ })
    for element in ${LUN_ID_TEMP_ARR[@]}
    do
        VGNAME_VALUE=`echo ${element} | ${MYAWK} -F "," '{print $1}'`
        LVMNAME_VALUE=`echo ${element} | ${MYAWK} -F "," '{print $2}'`
        MOUNTPOINT_PATH_VALUE=`echo ${element} | ${MYAWK} -F "," '{print $3}'`
        if [ "$VGNAME_VALUE" = "" ]
        then
            Exit 1 -log "[ERROR]:Vg Name is null."  -ret "Vg Name is null"
        fi
        if [ "$LVMNAME_VALUE" = "" ]
        then
            Exit 1 -log "[ERROR]:Lvm Name is null."  -ret "Lvm Name is null."
        fi
        if [ "$MOUNTPOINT_PATH_VALUE" = "" ]
        then
            Exit 1 -log "[ERROR]:Mount path is null."  -ret "Mount path is null."
        fi
        
        lvscan >> "${LOG_FILE_NAME}" 2>&1
        vgscan >> "${LOG_FILE_NAME}" 2>&1
        vgimport $VGNAME_VALUE >> "${LOG_FILE_NAME}" 2>&1
        vgchange -ay $VGNAME_VALUE >> "${LOG_FILE_NAME}" 2>&1
        if [ $? -ne 0 ]
        then
            Exit 1 -log "[ERROR]:vachange failed."  -ret "vachange failed."
        fi
        num=0
        while [ true ];do
            if [ num -ge 10 ];then
                Exit 1 -log "[ERROR]:$VGNAME_VALUE-$LVMNAME_VALUE not fountd"  -ret "$VGNAME_VALUE-$LVMNAME_VALUE not found."
            fi
            sleep 1
            ls /dev/mapper/ | grep -w "$VGNAME_VALUE-$LVMNAME_VALUE" >> "${LOG_FILE_NAME}" 2>&1
            if [ $? -ne 0 ];then
                let num+=1
                echo "echo $VGNAME_VALUE-$LVMNAME_VALUE not active" >> "${LOG_FILE_NAME}" 2>&1
                continue
            fi
            break
        done
        mount /dev/mapper/$VGNAME_VALUE-$LVMNAME_VALUE $MOUNTPOINT_PATH_VALUE >> "${LOG_FILE_NAME}" 2>&1
        if [ $? -ne 0 ]
        then
            Exit 1 -log "[ERROR]:mount device failed."  -ret "mount device failed."
        fi
        rm -rf $MOUNTPOINT_PATH_VALUE/*.pid
        rm -rf $MOUNTPOINT_PATH_VALUE/*.sock
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
        Exit 1 -log "[ERROR]:LUN_ID_PATH configure failed."  -ret "LunIdMountPath is empty"
    fi

    sh ${AGENT_ROOT_PATH}/bin/scandisk.sh ${AGENT_ROOT_PATH} >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Exit 1 -log "[ERROR]:Excute scan disk failed."  -ret "can disk failed."
    fi
    
    MountLVM

    [ -f ${TEMP_FILE_NAME} ] && rm -rf ${TEMP_FILE_NAME}
    
    mysqld_safe & service mysqld start >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Exit 1 -log "[ERROR]:Start mysql failed."  -ret "Start mysql failed."
    fi  
    
    Exit 0 -log "[INFO]:Finish start mysql."
}
Main