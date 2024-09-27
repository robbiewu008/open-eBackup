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

TEST_FRAME_ROOT_DIR=$(cd $(dirname ${BASH_SOURCE[0]});pwd)
COVERAGE_FILE_PATH="${TEST_FRAME_ROOT_DIR}/build/gcovr_report/coverage.html"
COVERAGE_TEMP_FILE_PATH="${TEST_FRAME_ROOT_DIR}/build/gcovr_report/tempfile"
TARGET_COVERAGE_FILE_PATH="${TEST_FRAME_ROOT_DIR}/conf/lcov_coverage.html"

SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${CURRENT_DIR}/../../../Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    echo "ERROR" "Module path no exist"
    exit 1
fi
MODULE_ROOT_PATH=$(cd "${MODULE_ROOT_PATH}"; pwd)
DT_UTILS_DIR=$(cd "${MODULE_ROOT_PATH}/dt_utils"; pwd)

sh ${DT_UTILS_DIR}/build_gmock.sh "$@"
if [ $? -ne 0 ];then
    exit 1
fi

function generate_report()
{
    if [ ! -f ${COVERAGE_FILE_PATH} ]; then
        echo "${COVERAGE_FILE_PATH} is not exist."
        return 1
    fi
    if [ ! -f ${TARGET_COVERAGE_FILE_PATH} ]; then
        echo "${TARGET_COVERAGE_FILE_PATH} is not exist."
        return 1
    fi

    # 从gcover生成报告中获取覆盖率
    line_cover=$(grep -A 5 "<th scope=\"row\">Lines:<\/th>" ${COVERAGE_FILE_PATH} | grep "<td class=\"coverage-*" \
        | awk -F ">" '{print $2}' | awk -F "%<" '{print $1}')
    if [ -z ${line_cover} ]; then
        echo "get line coverage failed."
        return 1
    fi

    if [ $(awk "BEGIN{print ${line_cover} < 70.0}") -eq 1 ]; then
        echo "line coverage less than 70 point."
        return 1
    fi

    func_cover=$(grep -A 5 "<th scope=\"row\">Functions:</th>" ${COVERAGE_FILE_PATH} | grep "<td class=\"coverage-*" \
        | awk -F ">" '{print $2}' | awk -F "%<" '{print $1}')
    if [ -z ${func_cover} ]; then
        echo "get function coverage failed."
        return 1
    fi

    branch_cover=$(grep -A 5 "<th scope=\"row\">Branches:</th>" ${COVERAGE_FILE_PATH} | grep "<td class=\"coverage-*" \
        | awk -F ">" '{print $2}' | awk -F "%<" '{print $1}')
    if [ -z ${branch_cover} ]; then
        echo "get branch coverage failed."
        return 1
    fi

    if [ $(awk "BEGIN{print ${branch_cover} < 48.0}") -eq 1 ]; then
        echo "branch coverage less than 50 point."
        return 1
    fi

    # 修改lcov报告覆盖率
    if [ -f ${COVERAGE_TEMP_FILE_PATH} ]; then
        rm -f ${COVERAGE_TEMP_FILE_PATH}
    fi
    awk '/<td class="headerCovTableEntryLo">/{print NR}' ${TARGET_COVERAGE_FILE_PATH} >> ${COVERAGE_TEMP_FILE_PATH}
    i=0
    while read line
    do
    echo $i
    i=`expr $i + 1`
    case $i in
        1)
            sed -i "${line}c\            <td class=\"headerCovTableEntryLo\">${line_cover} %</td>" ${TARGET_COVERAGE_FILE_PATH}
        ;;
        2)
            sed -i "${line}c\            <td class=\"headerCovTableEntryLo\">${func_cover} %</td>" ${TARGET_COVERAGE_FILE_PATH}
        ;;
        3)
            sed -i "${line}c\            <td class=\"headerCovTableEntryLo\">${branch_cover} %</td>" ${TARGET_COVERAGE_FILE_PATH}
    esac
    done < ${COVERAGE_TEMP_FILE_PATH}
}

function main()
{
    if [ "$1" == "clean" ]; then
        pushd ${TEST_FRAME_ROOT_DIR}/
        sh build.sh clean
        popd
        hdt clean ./
        return 0
    fi

    pushd ${TEST_FRAME_ROOT_DIR}/
    sh build.sh LLT
    if [ $? -ne 0 ]; then
        echo "build failed"
        popd
        exit 1
    fi
    popd

    cd ${TEST_FRAME_ROOT_DIR}
    if [ "$1" == "gdb" ]; then
        hdt run ./ -d on -s off -vvv
    else
        ARGS="--gtest_output=xml:report.xml $*"
        echo $ARGS
        hdt run ./ -vvv -s off --args="$ARGS"
        if [ $? != 0 ]; then
            exit 1
        fi
        # hdt report ./ --args="--exclude-unreachable-branches --exclude-throw-branches --filter=../../src"
        # 实际文件过滤
        hdt report ./ --args="--exclude-unreachable-branches --exclude-throw-branches --filter=../../src \
        -e '.+HostCommonService.h'\
        -e '.+JsonFileTool.h'\
        -e '.+HostCommonStruct.h'\
        -e '.+HostRestore.h'\
        -e '.+LvmSnapshot.h'\
        -e '.+HostIndex.h'\
        -e '.+HostIndex.cpp'\
        -e '.+ArchiveDownloadFile.h'\
        -e '.+ArchiveClient.h'\
        -e '.+CommonJobFactory.h'\
        -e '.+ShareResourceManager.h'\
        -e '.+ApplicationServiceDataType.h'\
        -e '.+HostLivemount.h'\
        -e '.+Win32Handler.cpp'\
        -e '.+Win32Handler.h'\
        -e '.+PosixHandler.cpp'\
        -e '.+ArchiveDownloadFile.cpp'\
        -e '.+HostBackup.cpp' \
        -e '.+HostCommonService.cpp' \
        -e '.+HostRestore.cpp' \
        -e '.+LvmSnapshotProvider.cpp' \
        -e '.+JfsSnapshotProvider.cpp' \
        -e '.+VssSnapshotProvider.cpp' \
        -e '.+OsIdentifier.cpp' \
        -e '.+HostCancelLivemount.cpp' \
        -e '.+ZfsSnapshotProvider.cpp' \
        -e '.+HostArchiveRestore.cpp'"
    fi
    generate_report
    if [ $? -ne 0 ]; then
        echo "failed"
        exit 1
    fi
    cd -
}

main "$@"