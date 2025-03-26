#!/bin/bash

TEST_ROOT_DIR=$(cd $(dirname ${BASH_SOURCE[0]});pwd)
COVERAGE_FILE_PATH="${TEST_ROOT_DIR}/build/gcovr_report/coverage.html"
COVERAGE_TEMP_FILE_PATH="${TEST_ROOT_DIR}/build/gcovr_report/tempfile"
TARGET_COVERAGE_FILE_PATH="${TEST_ROOT_DIR}/conf/lcov_coverage.html"
SCRIPT_PATH=$(dirname ${BASH_SOURCE[0]})
CURRENT_DIR=$(cd "${SCRIPT_PATH}"; pwd)
if [ -z "${MODULE_ROOT_PATH}" ];then
    MODULE_ROOT_PATH=${CURRENT_DIR}/../../Module
fi
if [ ! -d "${MODULE_ROOT_PATH}" ];then
    echo "ERROR" "Module path no exist"
    exit 1
fi
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

    if [ $(awk "BEGIN{print ${branch_cover} < 49.0}") -eq 1 ]; then
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
        pushd ${TEST_ROOT_DIR}/
        sh build.sh clean
        popd
        hdt clean ./
        return 0
    fi

    pushd ${TEST_ROOT_DIR}
    sh build.sh
    if [ $? -ne 0 ]; then
        echo "build failed"
        popd
        exit 1
    fi
    popd

    cd ${TEST_ROOT_DIR}
    if [ "$1" == "gdb" ]; then
        hdt run ./ -d on -s off -vvv
    else
        ARGS="--gtest_output=xml:report.xml $*"
        echo $ARGS
        hdt run ./ -vvv -s off --args="$ARGS"
        if [ $? != 0 ]; then
            exit 1
        fi
        hdt report ./ --args="--exclude-unreachable-branches --exclude-throw-branches --filter=../../../localhost_src --gcov-ignore-parse-errors \
        -e ../../../localhost_src/common/SmbProducerThread.cpp \
        -e ../../../localhost_src/common/NfsDirMtimeService.cpp \
        -e ../../../localhost_src/service/SmbFolderTraversal.cpp \
        -e ../../../localhost_src/common/SmbMetaProducer.cpp \
        -e ../../../localhost_src/common/DiffControlService.cpp \
        -e ../../../localhost_src/common/CommonService.cpp \
        -e ../../../localhost_src/interface/com_huawei_emeistor_dee_anti_ransomware_detect_so_Scanner.h \
        -e ../../../localhost_src/interface/com_huawei_emeistor_dee_anti_ransomware_detect_so_Scanner.cpp \
        -e ../../../localhost_src/service/ObjectStorage/ObjectMetaProducer.cpp \
        -e ../../../localhost_src/service/ObjectStorage/ObjectMetaReadThread.cpp \
        -e ../../../localhost_src/service/ObjectStorage/ObjectFolderTraversal.cpp \
        -e ../../../localhost_src/service/ObjectStorage/ObjectLogProcess.cpp"
    fi
    generate_report
    if [ $? -ne 0 ]; then
        echo "failed"
        exit 1
    fi
    cd -
}

main "$@"
