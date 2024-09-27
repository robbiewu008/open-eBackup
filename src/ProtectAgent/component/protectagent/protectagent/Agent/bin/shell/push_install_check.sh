#!/bin/sh
set +x


sysName=`uname -s`
if [ "${sysName}" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

BASE_PATH=""
if [ "${sysName}" = "AIX" ]; then
    BASE_PATH="$(cd "$(dirname $0)" && pwd)"
elif [ "${sysName}" = "SunOS" ]; then
    BASE_PATH=`cd $(dirname $0); pwd -P`
else
    BASE_PATH=$(cd "$(dirname $0)" && pwd)
fi

########## Repeated installation ################################
ipv4=""
ipv6=""
uuid=""

########## Specify the installation directory. ################################
CUSTOM_INSTLL_PATH=""
USER_NAME=""
RESULT_FILE="${BASE_PATH}/result.txt"

########## Check proxy information function entry ################################
GetLocalIp()
{
    local_ipv4=
    local_ipv6=
    # 1. 获取所有ip信息
    if [ "${sysName}" = "Linux" ]; then
        local_ipv4=`ip addr | grep -w "inet" | grep -v "127.0.0.1" |$AWK -F " " '{print $2}' | $AWK -F "/" '{print $1}'`
        local_ipv6=`ip addr | grep -w "inet6" | grep -v "::1" |$AWK -F " " '{print $2}' | $AWK -F "/" '{print $1}'`
    elif [ "${sysName}" = "AIX" ] || [ "${sysName}" = "SunOS" ]; then
        local_ipv4=`ifconfig -a | grep -w inet | grep -v "127.0.0.1" | ${AWK} '{print $2}'`
        local_ipv6=`ifconfig -a | grep -w inet6 | grep -v "::1"  | ${AWK} '{print $2}' | ${AWK} -F "%" '{print $1}' | ${AWK} -F "/" '{print $1}'`
    fi

    # 2. 组装ipv4结果
    for tmp_ipv4 in ${local_ipv4}; do
        if [ "${ipv4}" = "" ]; then
            ipv4="${tmp_ipv4}"
        else
            ipv4="${ipv4},${tmp_ipv4}"
        fi
    done

    # 3. 组装ipv6结果
    for tmp_ipv6 in ${local_ipv6}; do
        if [ "${ipv6}" = "" ]; then
            ipv6="${tmp_ipv6}"
        else
            ipv6="${ipv6},${tmp_ipv6}"
        fi
    done
    echo "ipv4=${ipv4}"
    echo "ipv6=${ipv6}"
}

GetHostUUID()
{
    if [ -f "/etc/HostSN/HostSN" ]; then
        uuid=`cat /etc/HostSN/HostSN`
    fi
    echo "uuid: ${uuid}"
}

WriteResult()
{
    echo "ipv4=${ipv4}" > "${BASE_PATH}/check_result.txt"
    echo "ipv6=${ipv6}" >> "${BASE_PATH}/check_result.txt"
    echo "uuid=${uuid}" >> "${BASE_PATH}/check_result.txt"
}

CheckProxy()
{
    # 1. 获取ip信息
    GetLocalIp

    # 2. 获取uuid信息
    GetHostUUID

    # 3. 输出到结果文件
    WriteResult
}

########## Check install path function entry ################################
CUSTOM_INSTLL_PATH_INVALID=1
CUSTOM_INSTLL_PATH_NOT_EXIST=2
CUSTOM_INSTLL_PATH_PERMISSION_HIGH=3
CUSTOM_INSTLL_PATH_PERMISSION_LOW=4

BLOCK_LIST="//|^/$|^/tmp$|^/tmp/.*|^/dev$|^/dev/.*|^/bin$|^/bin/.*|^/usr$|/usr/.*"

check_command_injection()
{
    expression='[|;&$><`\!]+'
    if [ "${SYS_NAME}" = "AIX" ]; then
        echo "$1" | grep -E "${expression}"
        if [ $? -eq 0 ]; then
            return 1
        fi
    else
        if [ "$1" =~ ${expression} ]; then
            echo "The param cannot contain special character(${expression})."
            return 1
        fi
    fi
    return 0
}

CheckInstallPath()
{
    CUSTOM_INSTLL_PATH="$1"
    #1. 删除历史结果文件
    if [ -f "${RESULT_FILE}" ]; then
        rm -f ${RESULT_FILE}
    fi

    #2. 校验特殊字符
    check_command_injection "${CUSTOM_INSTLL_PATH}"
    if [ $? -ne 0 ]; then
        echo "The customized installation directory contains special characters."
        echo "logDetail=${CUSTOM_INSTLL_PATH_INVALID}" >> ${RESULT_FILE}
        exit 1
    fi

    #3. realpath校验
    command -v realpath >/dev/null
    if [ $? -eq 0 ]; then
        realInstallPath=`realpath ${CUSTOM_INSTLL_PATH}`
    else
        if [ ! -d "${CUSTOM_INSTLL_PATH}" ]; then
            echo "logDetail=${CUSTOM_INSTLL_PATH_NOT_EXIST}" >> ${RESULT_FILE}
            exit 1
        fi
        realInstallPath=$(cd ${CUSTOM_INSTLL_PATH}; pwd)
    fi

    echo ${realInstallPath} | egrep ${BLOCK_LIST} > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "The customized installation directory invaild."
        echo "logDetail=${CUSTOM_INSTLL_PATH_INVALID}" >> ${RESULT_FILE}
    fi

    #4.校验路径是否存在
    if [ ! -d "${realInstallPath}" ]; then
        echo "The customized installation directory does not exist."
        echo "logDetail=${CUSTOM_INSTLL_PATH_NOT_EXIST}" >> ${RESULT_FILE}
        exit 1
    fi

    #5. 校验路径权限

    #5.1. 校验目录和父目录，对其他other用户是否有读和执行权限
    tmpDir=""
    index=2
    while [ 1 ]
    do
        subPath=`echo "${realInstallPath}" | $AWK -F "/" -v i="${index}" '{print $i}'`
        if [ "${subPath}" = "" ]; then
            break
        fi
        index=`expr $index + 1`

        tmpDir="${tmpDir}/${subPath}"
        command -v stat >/dev/null
        if [ $? -ne 0 ]; then
        #stat不可用使用perl
            accessRights=$(perl -e 'my @stat = stat($ARGV[0]); printf "%o\n", $stat[2] & 07777;' $tmpDir)
        else
            accessRights=`stat -c %a ${tmpDir}`
        fi
        tmpAccessRights=7
        if [ "${sysName}" = "SunOS" ]; then
            tmpAccessRights=`echo ${accessRights:2:1}`
        else
            tmpAccessRights=`expr substr "$accessRights" 3 1`
        fi
        if [ -z "${tmpAccessRights}" ] || [ ${tmpAccessRights} -lt 5 ]; then
            echo "Invalid low permission: $accessRights of path ${tmpDir}."
            echo "logDetail=${CUSTOM_INSTLL_PATH_PERMISSION_LOW}" >> ${RESULT_FILE}
            exit 1
        fi
    done

    currentDir=${realInstallPath}
    #5.2 校验用户、属组
    command -v stat >/dev/null
    if [ $? -ne 0 ]; then
        #stat不可用使用perl
        userName=$(perl -e 'my @stat = stat($ARGV[0]); my $uid = $stat[4]; my $username = getpwuid($uid); print "$username\n";' $currentDir)
        groupName=$(perl -e 'my @stat = stat($ARGV[0]); my $gid = $stat[5]; my $groupname = getgrgid($gid); print "$groupname\n";' $currentDir)
    else
        userName=`stat -c %U ${currentDir}`    
        groupName=`stat -c %G ${currentDir}`   
    fi

    if [ ${userName} != "root" ]; then
        echo "Invalid user: ${userName} or group: ${groupName}."
        echo "logDetail=${CUSTOM_INSTLL_PATH_PERMISSION_LOW}" >> ${RESULT_FILE}
        exit 1
    fi

    #5.3 校验权限
    command -v stat >/dev/null
    if [ $? -ne 0 ]; then
    #stat不可用使用perl
        accessRights=$(perl -e 'my @stat = stat($ARGV[0]); printf "%o\n", $stat[2] & 07777;' $currentDir)
    else
        accessRights=`stat -c %a ${currentDir}`
    fi
    result=$(find "${currentDir}" -type d \( ! -name . -prune \) \( -perm -g=w -o -perm -o=w \))
    if [ -n "$result" ]; then
        echo "Invalid high permission: $accessRights of path ${currentDir}."
        echo "logDetail=${CUSTOM_INSTLL_PATH_PERMISSION_HIGH}" >> ${RESULT_FILE}
        exit 1
    fi
    echo "Valid permission: $accessRights of path ${currentDir}."
    echo "logDetail=0" >> ${RESULT_FILE}
}

main()
{
    if [ $# -eq 0 ]; then
        CheckProxy
    else
        if [ $# -eq 2 ] && [ "$1" = "check_install_path" ]; then
            CheckInstallPath "$2"
        else
            echo "The parameter is incorrect."
            exit 1
        fi
    fi
}

main $*
