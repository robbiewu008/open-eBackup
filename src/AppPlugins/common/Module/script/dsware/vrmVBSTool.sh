#!/bin/bash
#
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
#
set +x

source "/etc/profile"

AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}
 
if [ -z "${AGENT_INSTALL_PATH}" ]; then
    AGENT_INSTALL_PATH="/opt"
fi

if [ ! -d "${AGENT_INSTALL_PATH}" ]; then
    echo "ERROR" "Agent Install Path [${AGENT_INSTALL_PATH}] : No such file or directory."
    return 1
fi

dswareTool_version="FusionStorage 8.0.1"
VIRT_ROOT_PATH="${AGENT_INSTALL_PATH}/DataBackup/ProtectClient/Plugins/VirtualizationPlugin"
VBSTOOL_INSTALL_DIR="${VIRT_ROOT_PATH}/vbstool/lib"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${VBSTOOL_INSTALL_DIR}
export PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/sbin:/root/bin:/usr/local/bin:$PATH
export DSWARE_JAR_PATH=$(find ${VIRT_ROOT_PATH}vbstool/lib -name "dsware-api*.jar" -type f -printf "%p")

if [ -f  "${VIRT_ROOT_PATH}"/bin/superlog.sh ]; then
    source "${VIRT_ROOT_PATH}"/bin/superlog.sh
else
    echo "cannot find file superlog.sh."
    exit 1
fi

export SHELL=/bin/bash

# 基本工作环境目录
DSWARE_BASE_DIR="/opt/dsware"

# vrmVBSTool operation result
SUCCESS=0
ERROR_CLUSTER=1
ERROR_MANAGER=2
ERROR_CLIENT=3
ERROR_FAILED=4
vrmVBSTool_result=$ERROR_CLIENT

# 获取JAVAHOME目录
JAVA_HOME=""
JAVA_ENV_DIR="/opt/javarunenv/"
JAVA_HOME_CONFIG_FILE="$JAVA_ENV_DIR/JAVA_HOME"
OLD_JAVA_HOME="$DSWARE_BASE_DIR/agent/jdk1.6.0"
OLD_JAVA_FILE="$OLD_JAVA_HOME/bin/java" 

# JRE所在路径 
APP_HOME="${VIRT_ROOT_PATH}/vbstool" 
DSWARE_API_JAR_FILE=$(ls $APP_HOME/lib/dsware-api*.jar 2>/dev/null) 
DSWARE_CLIENT_FILE1=$(ls $APP_HOME/conf/client_self.keystore 2>/dev/null)
DSWARE_CLIENT_FILE2=$(ls $APP_HOME/conf/client_trust.keystore 2>/dev/null)
DSWARE_API_PRO=$(ls $APP_HOME/conf/dsware-api.properties 2>/dev/null)
if [[ -z "$DSWARE_API_JAR_FILE" || -z "$DSWARE_CLIENT_FILE1" || -z "$DSWARE_CLIENT_FILE2" || -z "$DSWARE_API_PRO" ]]; then
    echo "DSwareAgent api package or configuration files dones not installed."
    echo "ResultCode=1003"
    exit 1
fi

# 需要启动的Java主程序（main方法类）
APP_MAINCLASS=com.dsware.vbs.tools.VrmVBSTools

# java虚拟机启动参数
JAVA_OPTS="-Xms256m -Xmx700m -XX:PermSize=64M -XX:MaxPermSize=128m" 

# 压缩文件大小上限
COMPRESSED_FILE_MAX_SIZE=300000000

# 拼凑完整的classpath参数，包括指定lib目录下所有的jar 
CLASSPATH=$APP_HOME:$APP_HOME/conf
for i in "$APP_HOME"/lib/*.*; do 
    CLASSPATH="$CLASSPATH":"$i" 
done

DSWARE_API_HOME="/opt/dsware/agent/vbstool/lib" 
if [[ -d ${DSWARE_API_HOME} ]];then
    for i in "$DSWARE_API_HOME"/dsware-api*.*; do 
        CLASSPATH="$CLASSPATH":"$i" 
    done
fi

######################################################################
#   FUNCTION   : vrmVBSTool_debug
#   DESCRIPTION: 在vrmVBSTool执行前后输出调试信息
#   CALLS      : NULL
#   CALLED BY  : NULL
#   INPUT      : 参数1: 调试信息
#   OUTPUT     : 无
#   RETURN     : 无
######################################################################
vrmVBSTool_debug()
{
     echo `date` $1
     return $?
}

######################################################################
#   FUNCTION   : vrmVBSTool_error
#   DESCRIPTION: 在vrmVBSTool参数检测后输出错误信息
#   CALLS      : NULL
#   CALLED BY  : NULL
#   INPUT      : 参数1: 错误信息
#   OUTPUT     : 无
#   RETURN     : 无
######################################################################
vrmVBSTool_error()
{
     echo [`date`] $1
     return $?
}

######################################################################
#   FUNCTION   : vrmVBSTool_operater
#   DESCRIPTION: 调用vrmVBSTool配置工具
#   CALLS      : NULL
#   CALLED BY  : NULL
#   INPUT      : 参数1: 调用参数
#   OUTPUT     : 无
#   RETURN     : 无
######################################################################
vrmVBSTool_operater()
{
  vrmVBSTool_debug "vrmVBSTool operation start: $param"
  if [ -d ${APP_HOME} ]; then
      cd "${APP_HOME}" > /dev/null
  else
      echo "${APP_HOME} not exist"
      return 1
  fi
  log_info "vrmVBSTool_operater: $JAVA_HOME/bin/java $JAVA_OPTS -classpath $CLASSPATH $APP_MAINCLASS $*"
  if [ -L "$JAVA_HOME/bin/java" ]; then
    log_error "symbol link File."
    return $ERROR_FAILED
  fi
  $JAVA_HOME/bin/java $JAVA_OPTS -classpath $CLASSPATH $APP_MAINCLASS "$@"
  ret=$?
  log_info "vrmVBSTool_operater: operation end result $ret"

  cd -> /dev/null
  
  vrmVBSTool_debug "vrmVBSTool operation end."
  return $ret
}

######################################################################
#   FUNCTION   : vrmVBSTool_version
#   DESCRIPTION: 显示vrmVBSTool配置工具版本
#   CALLS      : NULL
#   CALLED BY  : NULL
#   INPUT      : NULL
#   OUTPUT     : NULL
#   RETURN     : NULL
######################################################################
vrmVBSTool_version()
{
   echo "$dswareTool_version"
   return $SUCCESS
}

######################################################################
#   FUNCTION   : vbsTool_checkPPID
#   DESCRIPTION: 判断当前脚本是否是通过flock调用的，如果不是则退出
#   CALLS      : NULL
#   CALLED BY  : vbsTool_analyzeJavaHome
#   INPUT      : NULL
#   OUTPUT     : NULL
#   RETURN     : NULL
######################################################################
vbsTool_checkPPID()
{
    parePID=`ps -p $$ -o ppid | grep -v PPID`
    flockResult=`ps -p $parePID -o cmd | grep -v CMD | grep flock`
    if [[ ! -n ${flockResult} ]]; then
        #if return null, means parent process is not flock. need to be exit
        log_info "the shell only to be used by flock."
        exit 0
    fi
}

######################################################################
#   FUNCTION   : vbsTool_readJavaHome
#   DESCRIPTION: 从JAVA_HOME的配置文件读取当前设置的JAVA根目录
#   CALLS      : NULL
#   CALLED BY  : vbsTool_analyzeJavaHome
#   INPUT      : NULL
#   OUTPUT     : NULL
#   RETURN     : NULL
######################################################################
vbsTool_readJavaHome() 
{ 
    if [[ -f ${JAVA_HOME_CONFIG_FILE} ]]; then
        #if file is exist, then read content
        log_info "JAVAHOME file is exist"
        ##Judge timestamp is newer than dsware agent
        conf_st=`date +%s -r  $JAVA_HOME_CONFIG_FILE`
        rtnstr=`find /opt/dsware/ | grep j[dk\|re].*.tar.gz`
        for filename in $rtnstr
        do
            tar_st=`date +%s -r  $filename`
            if [[ $tar_st -gt $conf_st ]]; then
                log_info "JAVEHOME is too old,delete it"
                rm -rf ${JAVA_HOME_CONFIG_FILE}/.. 1>/dev/null
                return $ERROR_FAILED
            fi
        done
        if [ -L "${JAVA_HOME_CONFIG_FILE}" ]; then
            log_error "symbol link File."
            exit ${ERROR_FAILED}
        fi
        TMP_JAVA_HOME=`cat $JAVA_HOME_CONFIG_FILE`
        #exec java -version
        if [ -L "$TMP_JAVA_HOME/bin/java" ]; then
            log_error "symbol link File."
            exit ${ERROR_FAILED}
        fi
        $TMP_JAVA_HOME/bin/java -version 1>/dev/null 2>/dev/null
        if [[ 0 == $? ]]; then
            JAVA_HOME=$TMP_JAVA_HOME
            return $SUCCESS
        fi
        log_info "exec $TMP_JAVA_HOME/bin/java -version failed."
    fi
    return $ERROR_FAILED
}

######################################################################
#   FUNCTION   : vbsTool_unzip_JDK
#   DESCRIPTION: 解压JDK或JRE压缩包
#   CALLS      : NULL
#   CALLED BY  : vbsTool_analyzeJavaHome
#   INPUT      : filename:待解压文件全路径
#   OUTPUT     : NULL
#   RETURN     : 0:成功; 4:失败
######################################################################
vbsTool_unzip_JDK()
{
    filename=$1
    #if target dir is not exist, create it.
    if [[ ! -d ${JAVA_ENV_DIR} ]]; then
        #if dir is not exist, create it.
        log_info "javarunenv is not exist, will create it."
        mkdir -p $JAVA_ENV_DIR
    fi
    
    #check if the file is exist.
    if [[ ! -f ${filename} ]]; then
        log_info "file:$1 is not exist. unzipping is not executing."
        return $ERROR_FAILED
    fi
    
    #unzip the file.
    for (( VAR = 0; VAR < 3; ++VAR )); do
        fileSize=$(tar -tvf "${filename}" | awk 'BEGIN{sum=0}{sum+=$3}END{print int(sum)}')
        if [ $fileSize -gt $COMPRESSED_FILE_MAX_SIZE ]; then
            log_info "compressed file($filename) is too large"
            return $ERROR_FAILED
        fi
        tar -zmxvf $filename -C $JAVA_ENV_DIR 1>/dev/null
        #if exec cmd failed
        if [[ 0 != $? ]]; then
            log_info "unzip file($filename) failed"
            sleep 30s
        else
            break
        fi
    done
    
    chown -h root:root -R $JAVA_ENV_DIR
    
    #check if unzip success
    findResult=`find $JAVA_ENV_DIR | xargs ls -t| grep bin/java$`
    for javaPath in $findResult
    do
        $javaPath -version 1>/dev/null 2>/dev/null
        if [[ 0 == $? ]]; then
            JAVA_HOME=${javaPath%/bin/java}
            touch $filename 
            return $SUCCESS
        fi
        log_info "exec java -version failed."
    done
    #if command failed, means unzip jdk failed.
    return $ERROR_FAILED
}

######################################################################
#   FUNCTION   : vbsTool_analyzeJavaHome
#   DESCRIPTION: 同步JAVA根目录信息
#   CALLS      : vbsTool_checkPPID;vbsTool_readJavaHome;vbsTool_unzip
#   CALLED BY  : vbsTool_getJavaHome
#   INPUT      : NULL
#   OUTPUT     : NULL
#   RETURN     : 0:成功; 4:失败
######################################################################
vbsTool_analyzeJavaHome()
{
    #check parent process
    vbsTool_checkPPID
    #get first
    vbsTool_readJavaHome
    if [[ $SUCCESS == $? ]]; then
        log_info "read configfile successful, no need to unzip."
        exit $SUCCESS
    fi
    
    log_info "JAVA_HOME file is not exist,need analyze."
    rtnstr=`find /opt/dsware/ |xargs ls -t | grep j[dk\|re].*.tar.gz`
    for filename in $rtnstr
    do
        #check if the file is unzipping
        findResult=`ps -ef | grep tar.*.$filename | grep -v grep | awk '{print $2}'`
        for unzipProcess in $findResult
        do
            #kill Process
            log_info "process($unzipProcess) is unzipping $filename, will kill it."
            kill -9 $unzipProcess 1>/dev/null 2>/dev/null
            rm -rf $JAVA_ENV_DIR/* 1>/dev/null
        done
        
        log_info "begin to unzip file:$filename"
        vbsTool_unzip_JDK $filename
        if [[ $SUCCESS == $? ]]; then
            #write JAVA_HOME to config file
            echo $JAVA_HOME > $JAVA_HOME_CONFIG_FILE
            exit $SUCCESS
        fi
    done
    exit $ERROR_FAILED 
}

######################################################################
#   FUNCTION   : vbsTool_getJavaHome
#   DESCRIPTION: 获取JAVA的根目录
#   CALLS      : NULL
#   CALLED BY  : NULL
#   INPUT      : NULL
#   OUTPUT     : NULL
#   RETURN     : 0:成功; 4:失败
######################################################################
vbsTool_getJavaHome()
{
    if [[ ! -d ${DSWARE_BASE_DIR} ]]; then
        echo "DSwareAgent does not installed."
        echo "ResultCode=1000"
        exit 1
    fi
    vbsTool_readJavaHome
    if [[ $SUCCESS == $? ]]; then
        return $SUCCESS
    fi

    if [ ! -d $VIRT_ROOT_PATH/vbstool/tmp ]; then
        mkdir -p "$VIRT_ROOT_PATH/vbstool/tmp"
        chown -R root:rdadmin "$VIRT_ROOT_PATH/vbstool/tmp"
    fi
    for (( count=0; count<=120; count++ ))
    do
        #get file lock
        log_info "begin get file locker."
        flock -xn $VIRT_ROOT_PATH/vbstool/tmp/.vbcVRMToolFLock -c "$VIRT_ROOT_PATH/vbstool/vrmVBSTool.sh --analyzeJavaHome" 1>/dev/null
        if [[ 0 != $? ]]; then
            #if failed , sleep 1 sec try again
            sleep 1s
            log_info "get file locker failed."
            continue
        fi
        vbsTool_readJavaHome
        if [[ $SUCCESS == $? ]]; then
            return $SUCCESS
        fi
        log_info "get java home success."
    done
    return $ERROR_FAILED
}

######################################################################
#   FUNCTION   : vrmVBSTool_help
#   DESCRIPTION: 显示vrmVBSTool配置工具使用语法
#   CALLS      : NULL
#   CALLED BY  : NULL
#   INPUT      : NULL
#   OUTPUT     : NULL
#   RETURN     : NULL
######################################################################
vrmVBSTool_help()
{
    echo  "Usage:  `basename $0`"  
  
    #创建类操作
    echo  "    --op \"createBitmapVolume\" --dsaIp \"dswareAgentIp1,...,dswareAgentIp3\" --snapNameFrom \"snapNameFrom\" --snapNameTo \"snapNameTo\" --dswareFloatIP \"dswareManagementIp\" --volName \"volName\" --poolId \"poolID\""
    #删除类操作
    echo  "    --op \"deleteVolume\" --dsaIp \"dswareAgentIp1,...,dswareAgentIp3\" --dswareFloatIP \"dswareManagementIp\" --volName \"volumeName\""
    #查询类操作
    echo  "    --op \"queryAllBitmapVolume\" --dsaIp \"dswareAgentIp1,...,dswareAgentIp3\" --dswareFloatIP \"dswareManagementIp\" --poolId \"poolID\""  
    echo  "    --op \"queryBitmapVolume\" --dsaIp \"dswareAgentIp1,...,dswareAgentIp3\" --dswareFloatIP \"dswareManagementIp\" --volName \"volName\"  --poolId \"poolID\""  
    echo  "    --help"
    echo  "    --version"
    return $SUCCESS
}

if [ $# -lt "1" ]; then
    vrmVBSTool_error "Invalid argument."
    vrmVBSTool_help
    exit $ERROR_CLIENT
fi

verify_special_char "$@"
param="$@"
case "$1" in
    --help)
        vrmVBSTool_help
        vrmVBSTool_result=$?
        ;;

    --version)        
        vrmVBSTool_version
        vrmVBSTool_result=$?
        ;;
    --analyzeJavaHome)
        vbsTool_analyzeJavaHome
        vrmVBSTool_result=$?
        ;;
    --getJavaHome)
        vbsTool_getJavaHome
        vrmVBSTool_result=$?
        ;;
    --op)
        #检查参数数量必须大于2
        if [ $# -lt "2" ]; then
            vrmVBSTool_error "There lack arguments after command: `basename $0` \"--op\""
            vrmVBSTool_help
            vrmVBSTool_result=$ERROR_CLIENT
        else
            #操作分支
            vbsTool_getJavaHome
            if [[ 0 != $? ]]; then
                log_info "get java home failed."
                exit 1
            fi
            case "$2" in
                deleteVolume)
                    vrmVBSTool_operater "$@"
                    vrmVBSTool_result=$?
                    ;;
                createBitmapVolume)
                    vrmVBSTool_operater "$@"
                    vrmVBSTool_result=$?
                    ;;            
                    
                queryBitmapVolume)
                    vrmVBSTool_operater "$@"
                    vrmVBSTool_result=$?
                    ;;            
                    
                queryAllBitmapVolume)
                    vrmVBSTool_operater "$@"
                    vrmVBSTool_result=$?
                    ;;    
                *)
                    vrmVBSTool_error "The arguments are invalid, please refer to the help."
                    vrmVBSTool_help
                    vrmVBSTool_result=$ERROR_CLIENT
                    ;;
            esac
        fi
        ;;
    *)
        vrmVBSTool_error "The arguments are invalid, please refer to the help."
        vrmVBSTool_help
        vrmVBSTool_result=$ERROR_CLIENT
        ;;
esac

exit $vrmVBSTool_result
