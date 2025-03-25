#!/bin/sh
set +x
CURDIR=`dirname $0`
CURPATH=`readlink -f $CURDIR`
AGENT_ROOT_PATH="${CURPATH}/../"
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"

# 该脚本仅在LINUX下运行
LOG_FILE_NAME="${LOG_PATH}/config_dpc_flow_control.log"
TMP_FILE_NAME="${AGENT_ROOT_PATH}/stmp/roce_config.txt"

function config_flow_control()
{
	Log "config roce flow control begin."
    dpc_install_path=`ps -ef | grep 'fusionstorage/dpc/bin/dpc' | grep -v grep | awk '{print $8}'`
    dpc_install_path=`echo ${dpc_install_path}  | awk -F '/fusionstorage/dpc/bin/dpc' '{print $1}'`
	${dpc_install_path}/fusionstorage/agent/script/check_roce_config.sh 1>${TMP_FILE_NAME} 2>>$LOG_FILE_NAME
	if [ $? -ne 0 ]
	then
		Log "exec check_roce_config.sh failed."
		rm -rf ${TMP_FILE_NAME}
		return 1
	fi

	cat ${TMP_FILE_NAME} | grep 'result=0' 1>/dev/null 2>/dev/null
	if [ $? -eq 0 ]
	then
		Log "roce config is ok."
		rm -rf ${TMP_FILE_NAME}
		return 0
	fi
	
	rm -rf ${TMP_FILE_NAME}
	rdma_service_level=`cat ${dpc_install_path}/fusionstorage/agent/conf/network.cfg | grep g_rdma_service_level | awk -F '=' '{print $2}' | awk -F '"' '{print $2}'`
	if [ "${rdma_service_level}" = "" ]
	then
		Log "can not find g_rdma_service_level in /opt/fusionstorage/agent/conf/network.cfg."
	    return 1
	fi

	${dpc_install_path}/fusionstorage/agent/script/RoCE_monitor.sh ${rdma_service_level} 0 1>>${LOG_FILE_NAME} 2>&1
	if [ "${rdma_service_level}" = "" ]
	then
		Log "config roce flow control failed."
	    return 1
	fi
	Log "config roce flow control succ."
	return 0
}

function main()
{
    config_flow_control
	if [ $? -ne 0 ]
	then
	    exit 1
	fi
}

main "$@"