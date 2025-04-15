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

AGENT_ROOT_PATH=$1
PID=$2
PARAM_NUM=$3

. "${AGENT_ROOT_PATH}/sbin/agent_sbin_func.sh"
LOG_FILE_NAME=${LOG_PATH}/gethostos.log

SYS_NAME=`uname -s`

iOSType=
strCmd=

HOST_TYPE_WINDOWS=1   # Windows
HOST_TYPE_REDHAT=2    # RedHat
HOST_TYPE_HP_UX_IA=3  # HPUX IA
HOST_TYPE_SOLARIS=4   # SOLARIS
HOST_TYPE_AIX=5       # AIX
HOST_TYPE_SUSE=6      # SUSE
HOST_TYPE_ROCKY=7     # ROCKY
HOST_TYPE_OEL=8       # OEL
HOST_TYPE_ISOFT=9     # ISOFT
HOST_TYPE_CENTOS=10   # CentOS
HOST_TYPE_KYLIN=11    # Kylin
HOST_TYPE_NEOKYLIN=12 # NeoKylin
HOST_TYPE_OPENEULER=13 #openEuler
HOST_TYPE_UOS=14 # UnionTech OS Server
HOST_TYPE_UBUNTU=15 # Ubuntu
HOST_TYPE_DEBIAN=16 # Debian
HOST_TYPE_ASIANUX=17 # Asianux
HOST_TYPE_NFSCHINA=18 #NFSChina
HOST_TYPR_CEOS=19 #CEOS
HOST_TYPE_ANOLIS=20 #ANOLIS
HOST_TYPR_KYLINSEC=21 #kylinsec

GetHostOS()
{
    case ${SYS_NAME} in
    "Linux")
        if [ -f "/etc/oracle-release" ]; then
            iOSType=${HOST_TYPE_OEL}
            strCmd=`cat /etc/oracle-release 2>/dev/null | awk '{print $NF}'`
        elif [ -f "/etc/kylin-release" ]; then
            iOSType=${HOST_TYPE_KYLIN}
            strCmd=`cat /etc/kylin-release 2>/dev/null | awk -F '(' '{print $1}' | awk '{print $NF}'`
        elif [ -f "/etc/kylinsec-release" ]; then
            iOSType=${HOST_TYPR_KYLINSEC}
            strCmd=`cat /etc/kylinsec-release 2>/dev/null | awk -F '(' '{print $1}' | awk '{print $NF}'`
        elif [ -f "/etc/neokylin-release" ]; then
            iOSType=${HOST_TYPE_NEOKYLIN}
            strCmd=`cat /etc/neokylin-release 2>/dev/null | awk -F '(' '{print $1}' | awk '{print $NF}'`
        elif [ -f "/etc/centos-release" ]; then
            iOSType=${HOST_TYPE_CENTOS}
            strCmd=`cat /etc/centos-release 2>/dev/null | awk -F '(' '{print $1}' | awk '{print $NF}'`
        elif [ -f "/etc/SuSE-release" ]; then
            iOSType=${HOST_TYPE_SUSE}
            strCmd_first=`cat /etc/SuSE-release 2>/dev/null | awk  '$2 == "=" {print $3}' | head -1`
            strCmd_last=`cat /etc/SuSE-release 2>/dev/null | awk  '$2 == "=" {print $3}' | tail -1`
            strCmd="${strCmd_first}.${strCmd_last}"
        elif [ -f "/etc/isoft-release" ]; then
            iOSType=${HOST_TYPE_ISOFT}
            strCmd=`cat /etc/isoft-release | awk '{print$5}'`
        elif [ -f "/etc/iSoft-release" ]; then
            iOSType=${HOST_TYPE_ISOFT}
            strCmd=`cat /etc/iSoft-release | awk '{print$3}'`
        elif [ -f "/etc/asianux-release" ]; then
            iOSType=${HOST_TYPE_ASIANUX}
            strCmd=`cat /etc/asianux-release | awk '{print$3}'`
        elif [ -f "/etc/redhat-release" ]; then
            iOSType=${HOST_TYPE_REDHAT}
            strCmd=`cat /etc/redhat-release 2>/dev/null | awk -F '(' '{print $1}' | awk '{print $NF}'`
        elif [ -f "/etc/euleros-release" ]; then
            iOSType=${HOST_TYPE_CENTOS}
            strCmd=`cat /etc/euleros-release 2>/dev/null | awk -F '(' '{print $1}' | awk '{print $NF}'`
        elif [ -f "/etc/openEuler-release" ]; then
            iOSType=${HOST_TYPE_OPENEULER}
            strCmd=`cat /etc/openEuler-release 2>/dev/null | awk '{print $3}'`
        elif [ -n "`cat /etc/issue | grep 'Linx'`" ]; then
            iOSType=${HOST_TYPE_ROCKY}
            if [ -f "/etc/linx-release" ]; then
                strCmd=`cat /etc/linx-release 2>/dev/null | awk '{print $NF}' | awk -F '.' '{print $1\".\"$2}'`  # Rocky4
            elif [ -f "/etc/debian_version" ]; then
                strCmd=`cat /etc/debian_version 2>/dev/null | awk '{print $NF}' | awk -F '.' '{print $1\".\"$2}'` # Rocky6
            else 
                Log "Get Rocky version and ostype failed!"
            fi
        elif [ -n "`cat /etc/os-release 2>/dev/null | grep -i 'uos'`" ]; then
            iOSType=${HOST_TYPE_UOS}
            strCmd=`cat /etc/os-release | grep "VERSION_ID=" | awk -F '=' '{print $2}' | tr -d "\""`
        elif [ -n "`cat /etc/os-release 2>/dev/null | grep -i 'NFSChina'`" ]; then  # NFSChina
            iOSType=${HOST_TYPE_NFSCHINA}
            strCmd=`cat /etc/os-release | grep "VERSION_ID=" | awk -F '=' '{print $2}' | tr -d "\""`
        elif [ -n "`cat /etc/os-release 2>/dev/null | grep -i 'suse'`" ]; then    # SUSE 15
            iOSType=${HOST_TYPE_SUSE}
            strCmd=`cat /etc/os-release | grep "VERSION_ID=" | awk -F '=' '{print $2}' | tr -d "\""`
        elif [ -n "`cat /etc/issue | grep -i 'ubuntu'`" ]; then
            iOSType=${HOST_TYPE_UBUNTU}
            if [ -f "/etc/os-release" ]; then
                strCmd=`cat /etc/os-release | grep "VERSION_ID=" | awk -F '=' '{print $2}' | tr -d "\""`
            else
                strCmd=`cat /etc/issue | grep -i 'ubuntu' | awk '{print $2}'`
            fi
        elif [ -f "/etc/debian_version" ]; then
            iOSType=${HOST_TYPE_DEBIAN}
            strCmd=`cat /etc/debian_version 2>/dev/null `
        elif [ -f "/etc/hce-release" ]; then
            iOSType=${HOST_TYPE_CENTOS}
            strCmd=`cat /etc/hce-release 2>/dev/null | awk -F '(' '{print $1}' | awk '{print $NF}'`
        elif [ -f "/etc/anolis-release" ]; then
            iOSType=${HOST_TYPE_ANOLIS}
            strCmd=`cat /etc/anolis-release 2>/dev/null | awk -F '(' '{print $1}' | awk '{print $NF}'`
        elif [ -f "/etc/CEOS-release" ]; then
            iOSType=${HOST_TYPR_CEOS}
            strCmd=`cat /etc/openEuler-release 2>/dev/null | awk '{print $3}'`
        else
            Log "Get host version failed, no feature file!"
            exit 1
        fi
    ;;
    "HP-UX")
        iOSType=${HOST_TYPE_HP_UX_IA}
        strCmd=`uname -r | awk -F 'B.' '{print $NF}'`
    ;;
    "AIX")
        iOSType=${HOST_TYPE_AIX}
        strCmd=`uname -vr | awk '{print $2"."$1}'`
    ;;
    "SunOS")
        iOSType=${HOST_TYPE_SOLARIS}
        strCmd=`uname -a | nawk '{print $3}'`
    ;;
    *)
        Log "Get host version failed, unsupported system!"
        exit 1
    ;;
    esac
}

GetHostOS
Log "ostype:${iOSType} , version: ${strCmd} , path:${RESULT_FILE} "
echo ${iOSType} > "${RESULT_FILE}"
echo ${strCmd} >> "${RESULT_FILE}"
exit 0