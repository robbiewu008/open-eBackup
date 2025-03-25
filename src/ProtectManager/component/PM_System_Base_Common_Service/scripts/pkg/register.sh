#!/bin/bash
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

#获取下载链接和解压私钥
DOWNLOAD_LINK=$1
INSTALL_RESULT=0
# 获取安装类型
HOST_MIGRATION=$2

# 获取网络平面IP
AGENT_IP=$3

# pm net port
PM_NET_PORT=$4

# installPath
INSTALL_PATH=$5

# OPERATE_STEP start downloadStart downloadFinish registerStart
STEP=$6

# 是否SanClient
AGENT_TYPE=$7

# 最小空闲空间要求
FREE_SPACE_MIN=$8

# 安装私钥
PRIVATE_KEY=$9

# 安装包类型
PACKAGE_TYPE=${10}

#是否是业务IP
IS_BUSINESS_IP_EXIST=${11}

# Register directory
OPERATION_DIR=${INSTALL_PATH}/register
OPERATION_LOG=${INSTALL_PATH}/register/register.log
OPERATION_ERR_LOG=${INSTALL_PATH}/register/register_error.log
INSTALL_LOG_PATH=${INSTALL_PATH}/DataBackup/ProtectClient/ProtectClient-E/slog
THIRD_LOG_PATH=${INSTALL_PATH}/DataBackup/ProtectClient/ProtectClient-A/CDMClient/HWClientService/logs

# Register script directory
SCRIPT_DIR=/opt/script

AGENT_DIR=/opt/DataBackup/ProtectClient
AGENT_SANCLIENT_DIR=/opt/DataBackup/SanClient
PRODUCT_NAME=DataProtect
HEADER_FILE=/opt/script/header.txt
DOWNLOAD_NAME_PREFIX=client

SYSTEM_NAME=`uname -s`
if [ "$SYSTEM_NAME" = "SunOS" ]; then
    MYAWK=nawk
else
    MYAWK=awk
fi

PrintProcess()
{
    time=`date "+%Y-%m-%d %H:%M:%S"`
    echo "${time} $1 $2" >> "${OPERATION_LOG}"
}

# Make register directory
MakeRegisterDir()
{
    mkdir -p ${OPERATION_DIR}
    touch ${OPERATION_LOG}
    chmod 750 ${OPERATION_DIR}
    chmod 640 ${OPERATION_LOG}
    return 0
}

# Check if agent is installed
CheckIfInstalled()
{
    # 只有主机迁移参数为0时可以强制覆盖安装
    if [[ "$HOST_MIGRATION" = 0 ]]; then
    return 0
    fi
    if [ "${AGENT_TYPE}" = "general" ]; then
      if [ -d ${AGENT_DIR} ]; then
          return 1
      else
          return 0
      fi
    else
      if [ -d ${AGENT_SANCLIENT_DIR} ]; then
          return 1
      else
          return 0
      fi
    fi
}

CheckIfInstalledByPath()
{
    # 只有主机迁移参数为0时可以强制覆盖安装
    if [[ "$HOST_MIGRATION" != 0 ]]; then
      if [ "${AGENT_TYPE}" = "general" ]; then
        if [ -n "$(cat /etc/profile | grep 'DATA_BACKUP_AGENT_HOME')" ]; then
            return 1
        else
            return 0
        fi
      else
        if [ -n "$(cat /etc/profile | grep 'DATA_BACKUP_SANCLIENT_HOME')" ]; then
            return 1
        else
            return 0
        fi
      fi
    return 0
    fi
}

#  Check disk space
CheckFreeDiskSpace()
{
    # make sure at least FREE_SPACE_MIN(KB) space left
    CHECK_DIR=${INSTALL_PATH}

    if [ "${SYSTEM_NAME}" = "Linux" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | grep -n '' | ${MYAWK} 'END{print $4}'`
    elif [ "${SYSTEM_NAME}" = "AIX" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | grep -n '' | ${MYAWK} '{print $3}' | sed -n '2p'`
    elif [ "${SYSTEM_NAME}" = "HP-UX" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | grep -w 'free' | ${MYAWK} '{print $1}'`
    elif [ "${SYSTEM_NAME}" = "SunOS" ]; then
        FREE_SPACE=`df -k ${CHECK_DIR} | ${MYAWK} '{print $4}' | sed -n '2p'`
    fi

    if [[ ${FREE_SPACE_MIN} -gt ${FREE_SPACE} ]]; then
        return 1
    fi

    return 0
}

# Get usable pm ip that can be accessed
GetUsableIp()
{
    SEMICOLON=":"
    DOT="."
    DOWNLOAD_IP_LIST=${DOWNLOAD_LINK}
    DOWNLOAD_IP_LIST=${DOWNLOAD_IP_LIST#*https://[}
    DOWNLOAD_IP_LIST=${DOWNLOAD_IP_LIST%]:$PM_NET_PORT*}
    PMIP_TYPE=

    index=1
    count=0
    while [ 1 ]
    do
        IP_CHOOSED=`echo "${DOWNLOAD_IP_LIST}" | $MYAWK -F ',' -v i="$index" '{print $i}'`
        if [ "${IP_CHOOSED}" != "" ]; then
            echo ${IP_CHOOSED} | grep "\\${SEMICOLON}" >/dev/null 2>&1
            if [ 0 -eq $? ]; then
                PMIP_TYPE=IPV6
                if [ "${SYSTEM_NAME}" = "Solaris" ]; then
                    ping6 -s ${IP_CHOOSED} 56 1 > /dev/null 2>&1
                else
                    ping6 -c 1 -w 3 ${IP_CHOOSED} > /dev/null 2>&1
                fi
                if [ $? -eq 0 ]; then
                    DOWNLOAD_IP=${IP_CHOOSED}
                    break
                else
                    index=`expr $index + 1`
                fi
            fi
            echo ${IP_CHOOSED} | grep "\\${DOT}" >/dev/null 2>&1
            if [ 0 -eq $? ]; then
                PMIP_TYPE=IPV4
                if [ "${SYSTEM_NAME}" = "Solaris" ]; then
                    ping -s ${IP_CHOOSED} 56 1 > /dev/null 2>&1
                else
                    ping -c 1 -w 3 ${IP_CHOOSED} > /dev/null 2>&1
                fi
                if [ $? -eq 0 ]; then
                    DOWNLOAD_IP=${IP_CHOOSED}
                    break
                else
                    index=`expr $index + 1`
                fi
            fi
        else
            break
        fi
    done

    if [ "${IP_CHOOSED}" = "" ]; then
        return 1
    else
        return 0
    fi
}

# Check package is unique
CheckPacUnique()
{
    cd ${OPERATION_DIR}
    if [ "$SYSTEM_NAME" = "AIX" ] || [ "$SYSTEM_NAME" = "SunOS" ]; then
        PACKAGE_COUNT=`ls ${OPERATION_DIR} -l | grep ${DOWNLOAD_NAME_PREFIX} | wc -l`
        if [ ${PACKAGE_COUNT} -ne 1 ]; then
            return 1
        else
            PACKAGE_NAME=`ls | grep ${DOWNLOAD_NAME_PREFIX}`
            return 0
        fi
    fi

    PACKAGE_COUNT=`ls ${OPERATION_DIR} -l | grep ${DOWNLOAD_NAME_PREFIX} | wc -l`
    if [ ${PACKAGE_COUNT} -ne 1 ]; then
        return 1
    else
        PACKAGE_NAME=`ls | grep ${DOWNLOAD_NAME_PREFIX} | grep -oP '(?<=\_).*'`
        mv `ls | grep ${DOWNLOAD_NAME_PREFIX}` ${PACKAGE_NAME}
        return 0
    fi
}

# SHA256 sum verification
CheckShaValue()
{
    AGENT_ZIPFILE=${OPERATION_DIR}/${PACKAGE_NAME}
    if [ ! -f "${AGENT_ZIPFILE}" ]; then
        return 1
    fi
    SHA256_VALUE=`echo ${PACKAGE_NAME} | $MYAWK -F "_" '{print $NF}'`
    # unix dont support sha256sum a file
    if [ ${SYSTEM_NAME} = "AIX" ] || [ ${SYSTEM_NAME} = "HP-UX" ] || [ ${SYSTEM_NAME} = "SunOS" ]; then
        return 0
    fi
    # return if sha256 command not exists
    command -v sha256sum >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        return 0
    fi
    # verify the consistence of the sha256 value
    sha256sum ${AGENT_ZIPFILE} | $MYAWK -F " " '{print $1}' | grep ${SHA256_VALUE} >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        return 0
    else
        return 1
    fi
}

# Unzip package
UnzipPackage()
{
    cd ${OPERATION_DIR}
    if [ "$SYSTEM_NAME" = "SunOS" ]; then
        PACKAGE_ZIP_NAME=${PACKAGE_NAME}
        unzip ${PACKAGE_ZIP_NAME} >/dev/null 2>&1
        return $?
    else
        PACKAGE_ZIP_NAME=${PACKAGE_NAME%_${SHA256_VALUE}*}
        mv ${PACKAGE_NAME} ${PACKAGE_ZIP_NAME}
        if [ -z "$PACKAGE_TYPE" ] || [ "$PACKAGE_TYPE" = "zip" ]; then
            unzip ${OPERATION_DIR}/${PACKAGE_ZIP_NAME} >/dev/null 2>&1
            return $?
        elif [ "$PACKAGE_TYPE" = "tar" ]; then
            PrintProcess "tar"
           tar -xf "${OPERATION_DIR}/${PACKAGE_ZIP_NAME}" >/dev/null 2>&1
            return $?
        else
            echo "Unsupported package type: ${PACKAGE_TYPE}"
            return 1
        fi
    fi
}

# Check whether the install script exists
CheckInstallScript()
{
    cd ${OPERATION_DIR}
    AGENT_FOLDER=`ls ${OPERATION_DIR} -l | grep ${PRODUCT_NAME} | grep '^d' | $MYAWK '{print $NF}'`
    cd ${AGENT_FOLDER}
    if [ "$HOST_MIGRATION" = "0" ]; then
        PrintProcess "===============begin check host migration script===============" "$LINENO"
        if [ -f "host_migration.sh" ]; then
            chmod +x host_migration.sh
            return 0
        else
            PrintProcess "The host migration script does not exist." "$LINENO"
            return 1
        fi
    else
        PrintProcess "===============begin check install script===============" "$LINENO"
        if [ -f "install.sh" ]; then
            chmod +x install.sh
            return 0
        else
            PrintProcess "The install script does not exist." "$LINENO"
            return 1
        fi
    fi
}

CheckInstallAixScript()
{
    cd ${OPERATION_DIR}
    AGENT_FOLDER=`ls -l ${OPERATION_DIR}| grep ${PRODUCT_NAME} | grep '^d' | $MYAWK '{print $NF}'`
    cd ${AGENT_FOLDER}
    if [ "$HOST_MIGRATION" = "0" ]; then
        PrintProcess "===============begin check host migration script===============" "$LINENO"
        EXIST_SHELL_SH=`ls  |grep "host_migration.sh" |wc -l`
        if [ $EXIST_SHELL_SH != 0 ]; then
            chmod +x host_migration.sh
            return 0
        else
            PrintProcess "The host migration script does not exist." "$LINENO"
            return 1
        fi
    else
        PrintProcess "===============begin check install script===============" "$LINENO"
        EXIST_SHELL_SH=`ls |grep -v 'uninstall.sh' |grep 'install.sh' |wc -l`
        if [ $EXIST_SHELL_SH != 0 ]; then
            chmod +x install.sh
            return 0
        else
            PrintProcess "The install script does not exist." "$LINENO"
            return 1
        fi
    fi
}

# Install agent
InstallAgent()
{
    cd ${OPERATION_DIR}/${AGENT_FOLDER}
    PRIVATE_KEY=`echo ${PRIVATE_KEY} | $MYAWK '$1=$1'`
    if [ "$HOST_MIGRATION" = "0" ]; then
        PrintProcess "===============begin agent migrate===============" "$LINENO"
        echo "${PRIVATE_KEY}" | sh host_migration.sh -mode push >> "${OPERATION_LOG}"
        INSTALL_RESULT=$?
        unset PRIVATE_KEY
        return 0
    elif [ "$IS_BUSINESS_IP_EXIST" = "false" ]; then
      PrintProcess "===============begin auto register install without ip===============" "$LINENO"
              echo "${PRIVATE_KEY}" | sh install.sh -mode push >> "${OPERATION_LOG}"
              INSTALL_RESULT=$?
              unset PRIVATE_KEY
              return 0
    else
        PrintProcess "===============begin auto register install===============" "$LINENO"
        echo "${PRIVATE_KEY}" | sh install.sh -mode push "$AGENT_IP" >> "${OPERATION_LOG}"
        INSTALL_RESULT=$?
        unset PRIVATE_KEY
        return 0
    fi
}

# Clean register.sh
CleanRegister()
{
    if [ -d ${SCRIPT_DIR} ]; then
        rm -rf ${SCRIPT_DIR}
    fi

    rm -rf ${OPERATION_DIR}/DataProtect*
    errCode=$1
    if [ -z "$errCode" ]; then
      errCode=0
    fi
    PrintProcess "CleanRegister errCode." "$errCode"
    if [ -d ${INSTALL_LOG_PATH} ]; then
        mv ${OPERATION_LOG} ${INSTALL_LOG_PATH}
        if [ "$errCode" -ne "250" ]; then
          rm -rf ${OPERATION_DIR}
        fi
    fi

    if [ -d ${THIRD_LOG_PATH} ]; then
        mv ${OPERATION_LOG} ${THIRD_LOG_PATH}
        if [ "$errCode" -ne "250" ]; then
          rm -rf ${OPERATION_DIR}
        fi
    fi

    return 0
}
#################################################################################
##  main process
#################################################################################

# Step1: check
if [ "$STEP" = "start" ]; then
  MakeRegisterDir
  if [ $? -ne 0 ]; then
    PrintProcess "Make register directory fail." "$LINENO"
    CleanRegister
    exit 3
  fi
  PrintProcess "Make register directory success." "$LINENO"

  CheckIfInstalled
  if [ $? -ne 0 ]; then
    PrintProcess "Host Agent is already installed." "$LINENO"
    CleanRegister
    exit 24
  fi
  PrintProcess "Host Agent is ready to be installed." "$LINENO"

  CheckIfInstalledByPath
  if [ $? -ne 0 ]; then
    PrintProcess "Host Agent is already installed." "$LINENO"
    CleanRegister
    exit 26
  fi
  PrintProcess "Host Agent is ready to be installed." "$LINENO"

  CheckFreeDiskSpace
  if [ $? -ne 0 ]; then
    PrintProcess "Operation directory capacity is low." "$LINENO"
    CleanRegister
    exit 5
  fi
  PrintProcess "Operation directory capacity is enough." "$LINENO"
  exit 0
fi

# Step2: verifyPackage
if [ "$STEP" = "registerStart" ]; then
  CheckPacUnique
  if [ $? -ne 0 ]; then
      PrintProcess "Package is not unique." "$LINENO"
      CleanRegister
      exit 9
  fi
  PrintProcess "Package(${PACKAGE_NAME}) is unique." "$LINENO"

  CheckShaValue
  if [ $? -ne 0 ]; then
      PrintProcess "Package is not complete." "$LINENO"
      CleanRegister
      exit 10
  fi
  PrintProcess "Package(${PACKAGE_NAME}) check right." "$LINENO"

  # Step3: unzip
  UnzipPackage
  if [ $? -ne 0 ]; then
      PrintProcess "Unzip package fail." "$LINENO"
      CleanRegister
      exit 11
  fi
  PrintProcess "Unzip package(${PACKAGE_ZIP_NAME}) success." "$LINENO"

  # Step4: install
  if [ "$SYSTEM_NAME" = "AIX" ] || [ "$SYSTEM_NAME" = "SunOS" ]; then
     PrintProcess "CheckInstallAixScript" "$LINENO"
     CheckInstallAixScript
  else
     PrintProcess "CheckInstallScript" "$LINENO"
     CheckInstallScript
  fi
  if [ $? -ne 0 ]; then
      PrintProcess "Install script not exists."
      CleanRegister
      exit 12
  fi
  PrintProcess "Install script exists."

  InstallAgent
  if [ ${INSTALL_RESULT} -ne 0 ]; then
      PrintProcess "Install agent fail." "$LINENO"
      if [ ${INSTALL_RESULT} = 1 ]; then
          CleanRegister
          exit 86
      fi
      CleanRegister ${INSTALL_RESULT}
      exit ${INSTALL_RESULT}
  fi
  PrintProcess "Install agent success." "$LINENO"
  CleanRegister
  exit 0
fi
