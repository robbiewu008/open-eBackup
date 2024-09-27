#!/bin/sh
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

AGENT_ROOT_PATH=${AGENT_ROOT}
. "${AGENT_ROOT_PATH}/bin/shell/agent_sbin_func.sh"

ERROR_DRIVER_GET_OS_VERSION=3
check_os_version()
{
    # save os type in DRIVER_OS_VERSION from getOSVersion
    getOSVersion >> /dev/null
    if [ -z "${DRIVER_OS_VERSION}" ]
    then 
        echo "Get os version error."
        exit ${ERROR_DRIVER_GET_OS_VERSION}
    fi
}

check_os_version

CLEAN=0
SECUREC_FILE_HOME=${AGENT_ROOT}/platform/securec
DR_MAKE_FILE=${AGENT_ROOT}/build/driver/Makefile
DRV_SRV_DIR=${AGENT_ROOT}/src/src/driver/linux
DRV_SRV_SHARE_DIR=${AGENT_ROOT}/src/src/driver/share
DRV_DR_BASE_DIR=${AGENT_ROOT}/bin/driver/dr
DRV_BAK_BASE_DIR=${AGENT_ROOT}/bin/driver/bak
DRV_DR_BIN_DIR=${DRV_DR_BASE_DIR}/${DRIVER_OS_VERSION}
DRV_BAK_BIN_DIR=${DRV_BAK_BASE_DIR}/${DRIVER_OS_VERSION}
KO_FILE_NAME=iomirror.ko
DR_DRV_NAME=iomirror.ko
DR_BAK_NAME=ebkbackup.ko

cp_makefile_to_src_dir()
{
    cp ${DR_MAKE_FILE} ${DRV_SRV_DIR}
}

delete_makefile_in_src_dir()
{
    rm ${DRV_SRV_DIR}/Makefile
}

cp_ko_to_des_dir()
{
    cp ${DRV_SRV_DIR}/${KO_FILE_NAME} ${DRV_DR_BIN_DIR}/${DR_DRV_NAME}
    cp ${DRV_SRV_DIR}/${KO_FILE_NAME} ${DRV_BAK_BIN_DIR}/${DR_BAK_NAME}
}

delete_ko_in_des_dir()
{
    rm -rf ${DRV_DR_BASE_DIR}/*
    rm -rf ${DRV_BAK_BASE_DIR}/*
}

cp_securec_file_to_drv()
{
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/secureprintoutput_a.c  ${DRV_SRV_DIR}
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/securecutil.c          ${DRV_SRV_DIR}
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/memset_s.c             ${DRV_SRV_DIR}
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/memcpy_s.c             ${DRV_SRV_DIR}
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/sprintf_s.c            ${DRV_SRV_DIR}
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/snprintf_s.c           ${DRV_SRV_DIR}
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/vsprintf_s.c           ${DRV_SRV_DIR}
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/vsnprintf_s.c          ${DRV_SRV_DIR}
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/vsnprintf_s.c          ${DRV_SRV_DIR}
    /bin/cp -rf ${SECUREC_FILE_HOME}/src/strncpy_s.c            ${DRV_SRV_DIR}
}

delete_securec_file()
{
    /bin/rm -rf ${DRV_SRV_DIR}/secureprintoutput_a.c
    /bin/rm -rf ${DRV_SRV_DIR}/securecutil.c
    /bin/rm -rf ${DRV_SRV_DIR}/memset_s.c
    /bin/rm -rf ${DRV_SRV_DIR}/memcpy_s.c
    /bin/rm -rf ${DRV_SRV_DIR}/sprintf_s.c
    /bin/rm -rf ${DRV_SRV_DIR}/snprintf_s.c
    /bin/rm -rf ${DRV_SRV_DIR}/vsprintf_s.c
    /bin/rm -rf ${DRV_SRV_DIR}/vsnprintf_s.c
    /bin/rm -rf ${DRV_SRV_DIR}/strncpy_s.c
}

create_dir()
{
    mkdir -p ${DRV_DR_BIN_DIR}
    mkdir -p ${DRV_BAK_BIN_DIR}
}

make_init()
{
    if [ $# != 0 ]
    then
        if [ "$1" = "clean" ]
        then
            CLEAN=1
        else
            echo "Invalid make option, support clean only."
            exit 2
        fi
    fi
}

main_enter()
{
    if [ ${AGENT_ROOT:-0} = 0 ]
    then
        echo "Please source env.sh first"
        exit 2
    else
        create_dir
    fi

    cd ${DRV_SRV_DIR}

    if [ ${CLEAN} -eq 1 ]
    then
        make clean
        main_result=$?
        if [ ${main_result} != 0 ]
        then
            echo "#########################################################"
            echo "   Make clean driver failed."
            echo "#########################################################"

            exit ${main_result}
        fi

        delete_ko_in_des_dir
        delete_makefile_in_src_dir
        delete_securec_file
    else
        cp_makefile_to_src_dir
        cp_securec_file_to_drv
        # meet compile requirement, change source file name
        [ -f "${DRV_SRV_SHARE_DIR}/om_bitmap.cpp" ] && mv "${DRV_SRV_SHARE_DIR}/om_bitmap.cpp" "${DRV_SRV_SHARE_DIR}/om_bitmap.c"        

        make        
        main_result=$?
        if [ ${main_result} != 0 ]
        then
            echo "#########################################################"
            echo "   Make driver failed."
            echo "#########################################################"

            exit ${main_result}
        fi

        cp_ko_to_des_dir
    fi
}

echo "#########################################################"
echo "   Copyright (C), 2013-2014, Huawei Tech. Co., Ltd."
echo "   Start to compile Agent driver"
echo "#########################################################"
StartTime=`date '+%Y-%m-%d %H:%M:%S'`

echo ${CFLAGS}

make_init $*

main_enter
main_result=$?
if [ ${main_result} != 0 ]
then
    echo "#########################################################"
    echo "   Compile Agent driver failed."
    echo "#########################################################"

    exit ${main_result}
fi

EndTime=`date '+%Y-%m-%d %H:%M:%S'`
echo "#########################################################"
echo "   Compile Agent driver completed."
echo "   begin at ${StartTime}"
echo "   end   at ${EndTime}"
echo "#########################################################"

