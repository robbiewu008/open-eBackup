#!/bin/sh
# 可复用的公共操作，在此脚本进行封装
# *注意*：若要兼容bash和ksh，必需使用. 的方式引入common，并事先定义COMMON_PATH!
if [ -z "${COMMON_PATH}" ]; then
    COMMON_PATH="$(cd $(dirname ${BASH_SOURCE[0]}); pwd)"  # 该命令在bash中获取的才是common路径，ksh中为调用者路径
fi
SCRIPT_NAME=$(basename $0)
NAS_ROOT_DIR=$(cd "${COMMON_PATH}/../.."; pwd)

# 外部包存放路径
EXT_PKG_DOWNLOAD_PATH=${NAS_ROOT_DIR}/ext_pkg

# 依赖的DME框架包外层根目录名称
DME_ROOT_DIR_NAME=data_move_engine

# 依赖的DME框架Framework代码根目录名称
FRAMEWORK_ROOT_PATH=${NAS_ROOT_DIR}/${DME_ROOT_DIR_NAME}/DME_Framework

# 出包路径
NAS_PACKAGE_PATH=${NAS_ROOT_DIR}/output_pkg

# 系统类型
OS_TYPE=$(uname -s)
if [ "${OS_TYPE}" = "AIX" ]; then
    export OBJECT_MODE=64
    export CFLAGS=-maix64 && export CXXFLAGS=-maix64  # AIX Cmake必需先声明这两个变量，否则cmake的test会编译报错
fi

log_echo()
{
    local type="DEBUG"
    local message="$1"
    if [ "$#" -eq 2 ];then
       type="$1"
       message="$2"
    fi
    echo "[$(date "+%Y-%m-%d %H:%M:%S")][${type}][${SCRIPT_NAME}][$(whoami)] ${message}"
}
