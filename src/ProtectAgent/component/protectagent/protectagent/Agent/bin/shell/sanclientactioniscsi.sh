#!/bin/sh
set +x
 
AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
 
#for log
LOG_FILE_NAME="${LOG_PATH}/sanclientaction.log"

RESULT_FILE="${STMP_PATH}/${RESULT_TMP_FILE_PREFIX}${PID}"


# Log "********************************Start to execute the create lun********************************"
 
# global var, for kill monitor
PARAM_CONTENT=`ReadInputParam`
test -z "$PARAM_CONTENT"              && ExitWithError "PARAM_FILE"

#for GetValue
SanclientIqn=`GetValue "${PARAM_CONTENT}" sanclientiqn`
AgentIqn=`GetValue "${PARAM_CONTENT}" agentiqn`
FileioName=`GetValue "${PARAM_CONTENT}" fileioname`
FileioFullPathName=`GetValue "${PARAM_CONTENT}" fileiofullpathname`
Lunid=`GetValue "${PARAM_CONTENT}" lunid`
UnidirectionalAuthPwd=`GetValue "${PARAM_CONTENT}" unidirectionalAuthPwd`
SanclientIP=`GetValue "${PARAM_CONTENT}" sanclientIP`
FileIO=`targetcli /backstores/fileio ls`
FileCreate=`echo $FileIO | grep "$FileioName"`

SanclientIqn=${SanclientIqn%_*}

#取消acls自动映射
targetcli set global auto_add_mapped_luns=true

if [ "$FileCreate" == "" ]; then
    if [ -f "${FileioFullPathName}" ]; then
        Log "${FileioFullPathName} is exist."
        targetcli /backstores/fileio/ create name=${FileioName} file_or_dev=${FileioFullPathName} >> ${LOG_FILE_NAME} 2>&1
        if [ $? -ne 0 ]; then
            Log "Create fileio failed!"
            exit 1
        fi
    else
        Log "${FileioFullPathName} is not exist.create fileio failed."
        exit 1
    fi
fi

#创建iscsi通道
targetcli iscsi/ create ${SanclientIqn}
if [ $? -ne 0 ]; then
    targetcli /backstores/fileio/ delete ${FileioName}
    Log "Set server iqn failed!"
    exit 1
fi

#设置属性，启用登录认证
targetcli iscsi/${SanclientIqn}/tpg1/ set attribute generate_node_acls=0 >> ${LOG_FILE_NAME} 2>&1
if [ $? -ne 0 ]; then
    targetcli /backstores/fileio/ delete ${FileioName}
    Log "Set generate_node_acls failed!"
    exit 1
fi

targetcli iscsi/${SanclientIqn}/tpg1/ set attribute authentication=1 >> ${LOG_FILE_NAME} 2>&1
if [ $? -ne 0 ]; then
    targetcli /backstores/fileio/ delete ${FileioName}
    Log "Set authentication failed!"
    exit 1
fi

#创建白名单
targetcli iscsi/${SanclientIqn}/tpg1/acls/ create ${AgentIqn}
if [ $? -ne 0 ]; then
    targetcli /backstores/fileio/ delete ${FileioName}
    Log "Create agent iqn failed!"
    exit 1
fi

#添加port
isSetIP=`targetcli iscsi/${SanclientIqn}/tpg1/portals/ ls | grep ${SanclientIP}`
if [ "$isSetIP" != "" ]; then
    port=`targetcli iscsi/${SanclientIqn}/tpg1/portals/ ls | grep ${SanclientIP} | awk '{print $2}' | awk -F ':' '{print $2}'`
else
    port=3260
    port_status=`netstat -nlt | grep ${port} | wc -l`
    while [ ${port_status} -ne 0 ]
    do
        port=`expr ${port} + 1`
        port_status=`netstat -nlt | grep ${port} | wc -l`
    done
    targetcli iscsi/${SanclientIqn}/tpg1/portals/ create ${SanclientIP} ${port} >> ${LOG_FILE_NAME} 2>&1
    if [ $? -ne 0 ]; then
        targetcli /backstores/fileio/ delete ${FileioName}
        Log "Set ip and port failed!"
        exit 1
    fi
fi
Log "Port: ${port}"
echo "$port" >> "${RESULT_FILE}"

#设置单向认证信息，嵌入式使用expect免交互
/usr/bin/expect <<-EOF
    spawn targetcli
    expect ">"
    send "/iscsi/${SanclientIqn}/tpg1/acls/${AgentIqn}/ set auth userid=${AgentIqn}\r"
    expect ">"
    send "/iscsi/${SanclientIqn}/tpg1/acls/${AgentIqn}/ set auth password=${UnidirectionalAuthPwd}\r"
    expect ">"
    send "exit\r"
    expect "anyway:"
    send "exit\r"
    set timeout 30
    expect eof
EOF

if [ $? -ne 0 ]; then
    targetcli /backstores/fileio/ delete ${FileioName}
    Log "Set username or password failed!"
    exit 1
fi

#执行命令创建lun
if [ "$FileCreate" == "" ]; then
    targetcli iscsi/${SanclientIqn}/tpg1/luns/ create /backstores/fileio/${FileioName} lun=${Lunid}
    if [ $? -ne 0 ]; then
        targetcli /backstores/fileio/ delete ${FileioName}
        Log "Create Lun failed!"
        exit 1
    fi
fi