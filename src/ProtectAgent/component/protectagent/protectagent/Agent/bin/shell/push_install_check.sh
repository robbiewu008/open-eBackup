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

######### Check Dataturbo install ###############################
WHITELIST_CONTENT=

if [ "${sysName}" = "AIX" ]; then
    set -A WHITELIST_CONTENT    \
        "CentOS7,3.10.0,x86_64" \
        "CentOS8,4.18.0,x86_64" \
        "CentOS9,5.14.0,x86_64" \
        "SLES12SP4,4.12.14,x86_64"  \
        "SLES12SP5,4.12.14,x86_64"  \
        "OL6,4.*.*,x86_64"  \
        "OL7,4.14.35,x86_64"    \
        "OL7,5.*.*,x86_64"  \
        "Ubuntu20.04,5.*.*,x86_64"  \
        "Ubuntu22.04,5.*.*,x86_64"  \
        "openEuler22.03,5.10*,x86_64"   \
        "openEuler22.03,5.10*,aarch64"  \
        "EulerOS2.0,5.10*,x86_64"   \
        "EulerOS2.0,5.10*,aarch64"  \
        "Kylin Linux Advanced ServerV10,4.19.90,aarch64"
else
    declare -a WHITELIST_CONTENT
    WHITELIST_CONTENT[0]="CentOS7,3.10.0,x86_64"
    WHITELIST_CONTENT[1]="CentOS8,4.18.0,x86_64"
    WHITELIST_CONTENT[2]="CentOS9,5.14.0,x86_64"
    WHITELIST_CONTENT[3]="SLES12SP4,4.12.14,x86_64"
    WHITELIST_CONTENT[4]="SLES12SP5,4.12.14,x86_64"
    WHITELIST_CONTENT[5]="OL6,4.*.*,x86_64"
    WHITELIST_CONTENT[6]="OL7,4.14.35,x86_64"
    WHITELIST_CONTENT[7]="OL7,5.*.*,x86_64"
    WHITELIST_CONTENT[8]="Ubuntu20.04,5.*.*,x86_64"
    WHITELIST_CONTENT[9]="Ubuntu22.04,5.*.*,x86_64"
    WHITELIST_CONTENT[10]="openEuler22.03,5.10*,x86_64"
    WHITELIST_CONTENT[11]="openEuler22.03,5.10*,aarch64"
    WHITELIST_CONTENT[12]="EulerOS2.0,5.10*,x86_64"
    WHITELIST_CONTENT[13]="EulerOS2.0,5.10*,aarch64"
    WHITELIST_CONTENT[14]="Kylin Linux Advanced ServerV10,4.19.90,aarch64"
fi

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
CUSTOM_INSTLL_DATATURBO_INVALID=5

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

get_os_type_online()
{
    local os_type=""
    if [ -f "/etc/oracle-release" ]; then
        if [ $(grep -c -E "(Oracle Linux Server release [6,7])" /etc/oracle-release) -ne 0 ]; then
            mVersion=$(cat /etc/oracle-release | awk -F "." '{print $1}' | awk -F " " '{print $NF}')
            uVersion=$(cat /etc/oracle-release | awk -F "." '{print $2}' | awk -F " " '{print $1}')
	        if [ "${uVersion}" = "" ];then
                os_type="OL${mVersion}"0
            else
                os_type="OL${mVersion}${uVersion}"
            fi
        else
            echo "This OS is not supported."
            return 1
        fi
    elif [ -f "/etc/redhat-release" ]; then
        if [ $(grep -c -E "(Red Hat Enterprise Linux Server release [7,8,9]|Rocky Linux release [8,9]|Red Hat Enterprise Linux release [7,8,9]|CentOS release [7,8]|CentOS Linux release [7,8]|AlmaLinux release [8,9])" /etc/redhat-release) -ne 0 ]; then
            mVersion=$(cat /etc/redhat-release | awk -F "." '{print $1}' | awk -F " " '{print $NF}')
            uVersion=$(cat /etc/redhat-release | awk -F "." '{print $2}' | awk -F " " '{print $1}')
            if [ "${uVersion}" = "" ]; then
                os_type="CentOS${mVersion}"0
            else
                os_type="CentOS${mVersion}${uVersion}"
            fi
        else
            echo "This OS is not supported."
            return 1
        fi
    elif [ -f "/etc/SuSE-release" ]; then
        local vernum=$(cat /etc/SuSE-release | sed -n 's/^[[:blank:]]*VERSION[[:blank:]]*=[[:blank:]]*\([0-9][0-9]*\)[[:blank:]]*$/\1/p')
        local versp=$(cat /etc/SuSE-release | sed -n 's/^[[:blank:]]*PATCHLEVEL[[:blank:]]*=[[:blank:]]*\([0-9][0-9]*\)[[:blank:]]*$/\1/p')
        local version="SLES${vernum}SP${versp}"
        if [ $(echo "${version}" | grep -c -E "SLES12SP4|SLES12SP5") -ne 0 ]; then
            os_type="${version}"
        else
            echo "This OS is not supported. ${version}."
            return 1
        fi
    elif [ -f "/etc/os-release" ]; then
        local name=$(cat /etc/os-release |grep ^NAME | awk -F "=" '{print $2}' | sed 's/\"//g')
        local version_id=$(cat /etc/os-release |grep ^VERSION_ID | awk -F "=" '{print $2}' | sed 's/\"//g')
        local version="${name}${version_id}"
        if [ $(echo "${version}" | grep -c -E "(Ubuntu20.04|Ubuntu22.04|openEuler22.03|Kylin Linux Advanced ServerV10|SLES15.5|EulerOS2.0)") -ne 0 ]; then
            os_type="${version}"
        else
            echo "This OS is not supported. ${version}."
            return 1
        fi
    fi

    echo "${os_type}"
}

check_kernel_version()
{
    local os_type=""
    local cur_os_type="$(get_os_type_online)"
    local kernel_version=""
    
    for str in "${WHITELIST_CONTENT[@]}"
    do
		echo "check_kernel_version:${str}"
        os_type=$(echo "${str}" | awk -F "," '{print $1}')
        kernel_version=$(echo "${str}" | awk -F "," '{print $2}')
        arch_type=$(echo "${str}" | awk -F "," '{print $3}')
        if [ $(echo "${cur_os_type}" | grep -c -E "${os_type}") -eq 1 ] && \
           [ $(uname -r | grep -c -E "${kernel_version}") -eq 1 ] && \
           [ "${arch_type}" == "$(uname -p)" ]; then
            return 0
        fi
    done
    return 1
}

CheckDataturboInstall()
{
    if [ -f "${RESULT_FILE}" ]; then
        rm -f ${RESULT_FILE}
    fi

    check_kernel_version
    if [ $? -ne 0 ]; then
		echo "This OS with kernel version{$(uname -r)} is not supported."
        echo "logDetail=${CUSTOM_INSTLL_DATATURBO_INVALID}" >> ${RESULT_FILE}
		exit 1
    fi
    echo "logDetail=0" >> ${RESULT_FILE}
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
        elif [ $# -eq 1 ] && [ "$1" = "check_install_dataturbo" ]; then
            CheckDataturboInstall
        else
            echo "The parameter is incorrect."
            exit 1
        fi
    fi
}

main $*
