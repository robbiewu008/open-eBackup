#!/bin/sh
set +x
. "$2/bin/agent_thirdpartyfunc.sh"

function UnMountLVM()
{ 
    LUN_ID_TEMP_ARR=(${LunIdMountPath//###/ })
    for (( i=0 ; i<${#LUN_ID_TEMP_ARR[@]} ; i++ ))
    do
        for (( j=${#LUN_ID_TEMP_ARR[@]} - 1 ; j>i ; j-- ))
        do
            TEMP_NUM0=`echo ${LUN_ID_TEMP_ARR[j] } |tr -cd  / |wc -c`
            TEMP_NUM1=`echo ${LUN_ID_TEMP_ARR[j-1]} |tr -cd  / |wc -c `
            if  [[ $TEMP_NUM1 -lt $TEMP_NUM0 ]]
            then
                t=${LUN_ID_TEMP_ARR[j]}
                LUN_ID_TEMP_ARR[j]=${LUN_ID_TEMP_ARR[j-1]}
                LUN_ID_TEMP_ARR[j-1]=$t
            fi
        done
    done
    for element in ${LUN_ID_TEMP_ARR[@]}
    do
        VGNAME_VALUE=`echo ${element} | ${MYAWK} -F "," '{print $1}'`
        LVMNAME_VALUE=`echo ${element} | ${MYAWK} -F "," '{print $2}'`
        MOUNTPOINT_PATH_VALUE=`echo ${element} | ${MYAWK} -F "," '{print $3}'`
        if [ "$VGNAME_VALUE" = "" ]
        then
            Exit 1 -log "[ERROR]:Vg Name is null."  -ret "Vg Name is null."
        fi
        if [ "$LVMNAME_VALUE" = "" ]
        then
            Exit 1 -log "[ERROR]:Lvm Name is null." -ret "Lvm Name is null."
        fi
        if [ "$MOUNTPOINT_PATH_VALUE" = "" ]
        then
            Exit 1 -log "[ERROR]:Mount path is null." -ret "Mount path is null."
        fi
        
        fuser "$MOUNTPOINT_PATH_VALUE" >> "${LOG_FILE_NAME}" 2>&1
        lsof | grep "$MOUNTPOINT_PATH_VALUE" >> "${LOG_FILE_NAME}" 2>&1
        
        umount /dev/mapper/$VGNAME_VALUE-$LVMNAME_VALUE >> "${LOG_FILE_NAME}" 2>&1
        if [ $? -ne 0 ]
        then
            #continue
            Exit 1 -log "[ERROR]:unmount device failed." -ret "unmount device failed."
        fi
        vgexport $VGNAME_VALUE >> "${LOG_FILE_NAME}" 2>&1
        vgchange -an $VGNAME_VALUE >> "${LOG_FILE_NAME}" 2>&1 
        
    done
}

function Main()
{
    Log "[INFO]:Begin to stop mysql." 
    #exit 0
    
    GetValue "${INPUT_PARAMETER_LIST}" MountInfo
    LunIdMountPath=$ArgValue
    if [ "$LunIdMountPath" = "" ]
    then
        Exit 1 -log "[ERROR]:LUN_ID_PATH configure failed." -ret "LUN_ID_PATH configure failed."
    fi

    GetValue "${INPUT_PARAMETER_LIST}" MysqlUser
    MYSQL_USER=$ArgValue
    if [ "$MYSQL_USER" = "" ]
    then
        Exit 1 -log "[ERROR]:Get mysql user failed." -ret "Get mysql user failed."
    fi
    
    GetValue "${INPUT_PARAMETER_LIST}" MysqlPassword
    MYSQL_PASSWORD=$ArgValue
    if [ "$MYSQL_PASSWORD" = "" ]
    then
        Exit 1 -log "[ERROR]:Get mysql password failed." -ret "Get mysql password failed."
    fi
    
    which mysql
    if [ $? -ne 0 ]
    then
        Exit 1 -log "[INFO]:mysql is not isInstall." -ret "mysql is not isInstall"
    fi
    service mysql status | grep "not running" >>"${LOG_FILE_NAME}" 2>&1
    ret1=$?
    service mysqld status | grep "dead" >>"${LOG_FILE_NAME}" 2>&1
    ret2=$?
    if [[ $ret1 -eq 0 || $ret2 -eq 0 ]];then
        Log "[INFO]:Mysql is already stopped." 
    else  
        mysqladmin -u$MYSQL_USER -p$MYSQL_PASSWORD shutdown >> "${LOG_FILE_NAME}" 2>&1
        service mysql stop >> "${LOG_FILE_NAME}" 2>&1
        process_id=`ps -ef | grep "mysql" | grep -v "mysqlstop.sh" |${MYAWK} -F " " '{print $2}'` >> "${LOG_FILE_NAME}" 2>&1
        for i in $process_id
        do
            kill -9 $i >> "${LOG_FILE_NAME}" 2>&1
        done
    fi    

    UnMountLVM
    
    sh ${AGENT_ROOT_PATH}/bin/scandisk.sh ${AGENT_ROOT_PATH} >> "${LOG_FILE_NAME}" 2>&1
    if [ $? -ne 0 ]
    then
        Exit 1 -log "[ERROR]:Excute scan disk failed." -ret "Excute scan disk failed."
    fi
    
    [ -f ${TEMP_FILE_NAME} ] && rm -rf ${TEMP_FILE_NAME}

    Exit 0 -log "[INFO]:Finish stop mysql." 
}
Main
