#!/bin/bash
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

TEST_FRAME_ROOT_DIR=$(cd $(dirname ${BASH_SOURCE[0]});pwd)

export MODULE_ROOT_PATH=${TEST_FRAME_ROOT_DIR}/Module
PLUGIN_FRAMEWORK_LIB_LIBEVENT=${MODULE_ROOT_PATH}/third_open_src/libevent_rel/lib

export LD_LIBRARY_PATH="${PLUGIN_FRAMEWORK_LIB_LIBEVENT}"

VIRTUALIZATION_DIR=${TEST_FRAME_ROOT_DIR}

function main()
{
    pushd $VIRTUALIZATION_DIR
    if [ "$1" == "clean" ]; then
        hdt clean ./test
        return 0
    fi

    if [ -d "./test/build" ]; then
        hdt clean ./test
    fi

    # total build and run
    if [ "$1" == "" ]; then
        mkdir -p ./test/src
        cp -rf ./src/* ./test/src/
        hdt test ./test -p ${LD_LIBRARY_PATH} -c on -s off --args="--gtest_output=xml:report.xml"
        if [ $? != 0 ]; then
            return 1
        fi
        hdt report ./test --args="--exclude-unreachable-branches --exclude-throw-branches --filter=../src/.*\.cpp \
        -e '.+BackupJob.cpp'\
        -e '.+ArchiveRestoreJob.cpp'\
        -e '.+FusionStorageCleanFile.cpp'\
        -e '.+OpenStackConsistentSnapshot.cpp'\
        -e '.+RestoreIOTask.cpp'\
        -e '.+HcsCinderClient.cpp'\
        -e '.+NovaClient.cpp'\
        -e '.+NeutronClient.cpp'\
        -e '.+HCSProtectEngine.cpp'\
        -e '.+HcsResourceAccess.cpp'\
        -e '.+ApiOperator.cpp'"
        return 0
    fi

    hdt build ./test -c on -s off
    if [ $? != 0 ]; then
        return 1
    fi

    if [ "$1" == "gdb" ]; then
        hdt run ./test -d on -s off
        if [ $? != 0 ]; then
            return 1
        fi
    elif [ "$1" == "filter" ]; then
        hdt run ./test -a--gtest_filter=$2
        if [ $? != 0 ]; then
            return 1
        fi
    fi

    popd

    return 0
}

function generate_report()
{   
    echo ${VIRTUALIZATION_DIR}
    GCOVR_HTML_FILE_PATH="${VIRTUALIZATION_DIR}/test/build/gcovr_report/coverage.html"
    LCOV_HTML_FILE_PATH="${VIRTUALIZATION_DIR}/test/index.html"
    COVERAGE_TEMP_FILE_PATH="${VIRTUALIZATION_DIR}/test/temp"
    if [ ! -f ${COVERAGE_FILE_PATH} ]; then
        echo "${COVERAGE_FILE_PATH} is not exist."
        return 1
    fi
    if [ ! -f ${LCOV_HTML_FILE_PATH} ]; then
        echo "${LCOV_HTML_FILE_PATH} is not exist."
        return 1
    fi

    # 从gcover生成报告中获取覆盖率
    line_cover=$(grep -A 5 "<th scope=\"row\">Lines:<\/th>" ${GCOVR_HTML_FILE_PATH} | grep "<td class=\"coverage" \
        | awk -F ">" '{print $2}' | awk -F "%<" '{print $1}')
    if [ -z ${line_cover} ]; then
        echo "get line coverage failed."
        return 1
    fi

    func_cover=$(grep -A 5 "<th scope=\"row\">Functions:</th>" ${GCOVR_HTML_FILE_PATH} | grep "<td class=\"coverage" \
        | awk -F ">" '{print $2}' | awk -F "%<" '{print $1}')
    if [ -z ${func_cover} ]; then
        echo "get function coverage failed."
        return 1
    fi

    branch_cover=$(grep -A 5 "<th scope=\"row\">Branches:</th>" ${GCOVR_HTML_FILE_PATH} | grep "<td class=\"coverage" \
        | awk -F ">" '{print $2}' | awk -F "%<" '{print $1}')
    if [ -z ${branch_cover} ]; then
        echo "get branch coverage failed."
        return 1
    fi

    # 判断上库覆盖率条件
    line_cover_int=$(echo ${line_cover} | awk '{print int($0)}')
    branch_cover_int=$(echo ${branch_cover} | awk '{print int($0)}')
    if [ ${line_cover_int} -lt 70 ]; then
        echo "Line coverage must be greater than 70%."
        return 1
    fi
    if [ ${branch_cover_int} -lt 49 ]; then
        echo "Branche coverage must be greater than 50%."
        return 1
    fi

    # 修改lcov报告覆盖率
    if [ -f ${COVERAGE_TEMP_FILE_PATH} ]; then
        rm -f ${COVERAGE_TEMP_FILE_PATH}
    fi
    awk '/<td class="headerCovTableEntry.*">/{print NR}' ${LCOV_HTML_FILE_PATH} >> ${COVERAGE_TEMP_FILE_PATH}
    i=0
    while read line
    do
    i=`expr $i + 1`
    case $i in
        3)
            sed -i "${line}c\            <td class=\"headerCovTableEntryLo\">${line_cover} %</td>" ${LCOV_HTML_FILE_PATH}
        ;;
        6)
            sed -i "${line}c\            <td class=\"headerCovTableEntryLo\">${func_cover} %</td>" ${LCOV_HTML_FILE_PATH}
        ;;
        9)
            sed -i "${line}c\            <td class=\"headerCovTableEntryLo\">${branch_cover} %</td>" ${LCOV_HTML_FILE_PATH}
    esac
    done < ${COVERAGE_TEMP_FILE_PATH}
    return 0
}

main "$@"

if [ $? != 0 ]; then
    exit 1
fi
generate_report