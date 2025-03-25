#!/bin/sh
# 
#  This file is a part of the open-eBackup project.
#  This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#  If a copy of the MPL was not distributed with this file, You can obtain one at
#  http://mozilla.org/MPL/2.0/.
# 
#  Copyright (c) [2024] Huawei Technologies Co.,Ltd.
# 
#  THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
#  EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
#  MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
set +x
#@function: query oracle rac info
AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3
. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"

#********************************define these for local script********************************
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for log
LOG_FILE_NAME="${LOG_PATH}/oracleracinfo.log"
#for GetOracleVersion
ORA_VERSION=
PREVERSION=
VERSION=
# global var, for kill monitor
#********************************define these for local script********************************
# define for the function GetOraUserByInstName
ORA_DB_USER=
ORA_GRID_USER=

CheckOracleIsInstall
RST=$?
if [ $RST -ne 0 ]; then
    exit $RST
fi

# get cluster configuration
GetOracleCluster
echo "ClusterType;${DBISCLUSTER}" > "${RESULT_FILE}"
if [ ${DBISCLUSTER} -ne 1 ]; then
    Log "This is not rac node, no need get more info."
    exit 0
fi

GetOracleUser
GetGridHomePath
GetOracleVersion

GRID_HOME=${IN_GRID_HOME}
Log "Get GRID_HOME=${GRID_HOME}."

# get ClusterName
ClusterName=`su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}${GRID_HOME}/bin/cemutlo -n"`
Log "Get ClusterName=${ClusterName}."
echo "ClusterName;${ClusterName}" >> "${RESULT_FILE}"


ScanIPName=
ScanIp=
GetClusterInfo()
{
    if [ -z "${ORA_GRID_USER}" ];then
            GetOracleUser
    fi

    scanConfig=`su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}srvctl config scan"`
    if [ "$scanConfig" = "" ]; then
            echo "get scanConfig fail"
            exit 1
    fi
    if [ `RDsubstr ${ORA_VERSION} 1 2` -gt 11 ]; then
        #ScanIPName
        ScanIPName=`echo "$scanConfig"| grep "SCAN name:" | $MYAWK -F "," '{print $1}'| $MYAWK -F ": " '{print $2}'`

        #ScanIp
        ScanIp=`echo "$scanConfig"| grep "VIP:" | $MYAWK -F ": " '{print $2}'`
    else
        #ScanIPName
        ScanIPName=`echo "$scanConfig" | grep "SCAN VIP" |$MYAWK -F "IP: /" '{print $2}' | $MYAWK -F "/" '{print $1}'`

        #ScanIp
        ScanIp=`echo "$scanConfig" | grep "SCAN VIP" |$MYAWK -F "IP: /" '{print $2}' | $MYAWK -F "/" '{print $2}'`
    fi
    Log "ScanIPName=$ScanIPName, ScanIp=$ScanIp"
}
GetClusterInfo

# get clusterIP
if [ "${ScanIPName}" = "" ]; then
    Log "Get cluster IP Name failed."
    exit 1
fi

if [ "${ScanIp}" = "" ]; then
    Log "Get cluster IP by clustername ${ScanIPName} failed."
    exit 1
fi
echo "ClusterIP;${ScanIp}" >> "${RESULT_FILE}"

ClusterHostInfo=`su - ${ORA_GRID_USER} ${GRID_SHELLTYPE} -c "${EXPORT_GRID_ENV}${GRID_HOME}/bin/crsctl status server | grep NAME" | $MYAWK -F = '{print $2}'`
Log "Get ClusterHostInfo=${ClusterHostInfo}."
# node ip and hostname
for node_name in `echo ${ClusterHostInfo}`; do
    if [ "${SYS_NAME}" = "AIX" ]; then
        node_ip=`ping -c 1 ${node_name} | head -1 | awk -F '(' '{print $2}' | awk -F ')' '{print $1}'`
    else
        node_ip=`ping -c 1 ${node_name} | sed '1{s/[^(]*(//;s/).*//;q}'`
    fi
    echo "ClusterIPHost;${node_name};${node_ip}" >> "${RESULT_FILE}"
    Log "Get ClusterIPHost=${node_name},${node_ip}"
done
Log "Get Oracle Cluster Info Succ."

#chmod 640 RESULT_FILE
if [ -f "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

exit 0