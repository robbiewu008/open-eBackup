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

############################################################################################
#program name:          oracleasm.sh
#function:              query oracle asm information
#function and description:  
# function              description
# rework:               First Programming
############################################################################################
AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
. "${AGENT_ROOT_PATH}/sbin/oraclefunc.sh"

#===define these for the functions of agent_sbin_func.sh and oraclefunc.sh===
#for GetUserShellType
ORACLE_SHELLTYPE=
GRID_SHELLTYPE=
RDADMIN_SHELLTYPE=
#for log
LOG_FILE_NAME="${LOG_PATH}/oracleasm.log"
#for GetOracleVersion
ORA_VERSION=
ORCLVESION="${STMP_PATH}/ORCV${PID}.txt"
SQLEXIT="${STMP_PATH}/EXITSQL${PID}.sql"
#for GetClusterType
DBISCLUSTER=0
#for GetOracleBasePath,GetOracleHomePath
IN_ORACLE_BASE=
IN_ORACLE_HOME=
# oracle temp file
ORATMPINFO="${STMP_PATH}/ORATMPINFO${PID}.txt"

CheckOracleIsInstall
RST=$?
if [ $RST -ne 0 ]; then
    exit $RST
fi

GetOracleUser
#get user shell type
GetUserShellType ${ORA_DB_USER} ${ORA_GRID_USER}

#get oracle base path
GetOracleBasePath ${ORA_DB_USER}

#get oracle home path
GetOracleHomePath ${ORA_DB_USER}

#get Oracle version
GetOracleVersion ${ORA_DB_USER}
VERSION=`echo $PREVERSION | tr -d '.'`
# get cluster flat
GetOracleCluster

CheckAsmAuth()
{
    local INST_NAME=$1
    CheckInstSQL="${STMP_PATH}/CheckInst${PID}.sql"
    CheckInstSQLRST="${STMP_PATH}/CheckInst${PID}.txt"
    echo "exit;" >> ${CheckInstSQL}
    
    ASMExeSql "" "" "${CheckInstSQL}" "${CheckInstSQLRST}" "${INST_NAME}" 10 1 "${ORA_GRID_USER}" >> "${LOG_FILE_NAME}" 2>&1
    RET_CODE=$?
    DeleteFile "${CheckInstSQL}" "${CheckInstSQLRST}"
    if [ "$RET_CODE" -ne "0" ]; then
        Log "Check ${INST_NAME} OS auth type failed."
        return 0
    fi
    Log "Check ${INST_NAME} OS auth type succ."
    return 1
}

Log "Begin to read oracle asm information."
DeleteFile "${RESULT_FILE}"

# check ASM instance
ASM_INST=`ps -ef | grep asm_...._+ | grep -v grep | awk -F_ '{print $NF}' | uniq`
if [ ! -z "$ASM_INST" ]; then
    CheckAsmAuth "${ASM_INST}"
    AuthType=$?
    Log "${ASM_INST};${AuthType};${DBISCLUSTER}"
    echo "${ASM_INST};${AuthType};${DBISCLUSTER}" >> "${RESULT_FILE}"
fi
Log "Get oracle asm succ."

#chmod 640 RESULT_FILE
if [ -f "${RESULT_FILE}" ]; then
    chmod 640 "${RESULT_FILE}"
fi

exit 0
