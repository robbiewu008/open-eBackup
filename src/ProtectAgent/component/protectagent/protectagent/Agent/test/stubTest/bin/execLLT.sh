#! /bin/sh

AGENT_ROOT="${HOME}/Agent"
AGENT_TEST_BIN="${AGENT_ROOT}/test/stubTest/bin"
AGENT_TEST_LCOV_HTML_DIR="${AGENT_ROOT}/test/stubTest/html"
AGENT_TEST_GCOVR_HTML_DIR="${AGENT_ROOT}/test/stubTest/gcovrhtml"
GCOVR_HTML_FILE_PATH="${AGENT_TEST_GCOVR_HTML_DIR}/index.html"
LCOV_HTML_FILE_PATH="${AGENT_TEST_LCOV_HTML_DIR}/index.html"
COVERAGE_TEMP_FILE_PATH="${AGENT_TEST_BIN}/tempfile"
LLT_RESULT="${AGENT_TEST_BIN}/llt_result"
export LD_LIBRARY_PATH=${AGENT_ROOT}/bin:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${AGENT_ROOT}/open_src/libevent/.libs/lib:${LD_LIBRARY_PATH}

function generate_lcov_report()
{
    export COVERAGE_FILE_TMP=total_tmp.info
    export COVERAGE_FILE=total.info
    lcov --rc lcov_branch_coverage=1 -c -d ${AGENT_ROOT} -o ${COVERAGE_FILE_TMP}
    lcov --rc lcov_branch_coverage=1 -e ${COVERAGE_FILE_TMP} \
    "*src/src/agent/*"                                      \
    "*src/src/alarm/*"                                      \
    "*src/src/apps/appprotect/AppProtectService.cpp"                          \
    "*src/src/apps/appprotect/plugininterface/JobServiceHandler.cpp"                          \
    "*src/src/apps/appprotect/plugininterface/PluginRegisterHandler.cpp"                          \
    "*src/src/apps/appprotect/plugininterface/SecurityServiceHandler.cpp"                          \
    "*src/src/apps/appprotect/plugininterface/ShareResourceHandler.cpp"                          \
    "*src/src/apps/dws/XBSAClient/DataConversion.cpp"                          \
    "*src/src/apps/dws/XBSAClient/File.cpp"                          \
    "*src/src/apps/dws/XBSAClient/ThriftClientMgr.cpp"                          \
    "*src/src/apps/dws/XBSAClient/xbsa.cpp"                          \
    "*src/src/apps/dws/XBSACom/*"                          \
    "*src/src/apps/dws/XBSAServer/DwsDppClient/*"                          \
    "*src/src/apps/dws/XBSAServer/BsaDb.cpp"                          \
    "*src/src/apps/dws/XBSAServer/BsaIntfAdaptor.cpp"                          \
    "*src/src/apps/dws/XBSAServer/BsaMountManager.cpp"                          \
    "*src/src/apps/dws/XBSAServer/BSAServiceHandler.cpp"                          \
    "*src/src/apps/dws/XBSAServer/BsaSession.cpp"                          \
    "*src/src/apps/dws/XBSAServer/BsaSessionManager.cpp"                          \
    "*src/src/apps/dws/XBSAServer/BsaTransManager.cpp"                          \
    "*src/src/apps/dws/XBSAServer/BsaTransState.cpp"                          \
    "*src/src/apps/dws/XBSAServer/CTimer.cpp"                          \
    "*src/src/apps/dws/XBSAServer/ThriftServer.cpp"                          \
    "*src/src/common/*"                                     \
    "*src/src/host/host.cpp"                                       \
    "*src/src/message/*"                                     \
    "*src/src/pluginfx/*"                                     \
    "*src/src/plugins/appprotect/*"                          \
    "*src/src/plugins/dws/*"                          \
    "*src/src/rootexec/*"                                   \
    "*src/src/securecom/*"                                  \
    "*src/src/servicecenter/*"                                  \
    "*src/src/taskmanager/externaljob/*"                \
    -o ${COVERAGE_FILE} > /etc/null 2>&1

    genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${AGENT_TEST_LCOV_HTML_DIR}
    # genhtml --rc genhtml_branch_coverage=1 ${COVERAGE_FILE} -o ${AGENT_TEST_LCOV_HTML_DIR} >> ${LLT_RESULT} 2>&1
    rm -rf ${COVERAGE_FILE_TMP}
}

function generate_report()
{
    if [ ! -f ${GCOVR_HTML_FILE_PATH} ]; then
        echo "${GCOVR_HTML_FILE_PATH} is not exist."
        exit 1
    fi
    if [ ! -f ${LCOV_HTML_FILE_PATH} ]; then
        echo "${LCOV_HTML_FILE_PATH} is not exist."
        exit 1
    fi

    # 从gcover生成报告中获取覆盖率
    line_cover=$(grep -A 5 "<th scope=\"row\">Lines:<\/th>" ${GCOVR_HTML_FILE_PATH} | grep "<td class=\"coverage" \
        | awk -F ">" '{print $2}' | awk -F "%<" '{print $1}')
    if [ -z ${line_cover} ]; then
        echo "get line coverage failed."
        exit 1
    fi

    func_cover=$(grep -A 5 "<th scope=\"row\">Functions:</th>" ${GCOVR_HTML_FILE_PATH} | grep "<td class=\"coverage" \
        | awk -F ">" '{print $2}' | awk -F "%<" '{print $1}')
    if [ -z ${func_cover} ]; then
        echo "get function coverage failed."
        exit 1
    fi

    branch_cover=$(grep -A 5 "<th scope=\"row\">Branches:</th>" ${GCOVR_HTML_FILE_PATH} | grep "<td class=\"coverage" \
        | awk -F ">" '{print $2}' | awk -F "%<" '{print $1}')
    if [ -z ${branch_cover} ]; then
        echo "get branch coverage failed."
        exit 1
    fi

    echo -e "Summary coverage rate:\n  lines......: ${line_cover}%\n "\
        "functions..: ${func_cover}%\n  branches...: ${branch_cover}%"

    # 判断上库覆盖率条件
    line_cover_int=$(echo ${line_cover} | awk '{print int($0)}')
    branch_cover_int=$(echo ${branch_cover} | awk '{print int($0)}')
    if [ ${line_cover_int} -lt 70 ]; then
        echo "Line coverage must be greater than 70%."
    fi
    if [ ${branch_cover_int} -lt 44 ]; then
        echo "Branche coverage must be greater than 44%."
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
}

if [ -z ${AGENT_ROOT} ] || [ ! -d ${AGENT_ROOT} ]; then
    echo "Agent path not existence."
    exit 1
fi

# 运行可执行文件 产生gcda文件
sh ${AGENT_TEST_BIN}/run.sh > ${LLT_RESULT} 2>&1

echo "--------------Start to filter fail------------------"
cat ${LLT_RESULT} | grep -C 50 "Segmentation fault"
if [ $? -eq 0 ]; then
    echo "There is Segmentation fault in exec result."
    exit 1
fi
cat ${LLT_RESULT} | grep -C 50 "\[  FAILED  \]"
if [ $? -eq 0 ]; then
    echo "There is llt fail in exec result."
	exit 1
fi
echo "--------------Filter fail finish------------------"

find ${AGENT_ROOT}/test/stubTest/obj -name "*.gcda" | xargs rm -rf

# 检查是否存在lcov报告目录及文件
if [ ! -d ${AGENT_TEST_LCOV_HTML_DIR} ] || [ ! -f ${LCOV_HTML_FILE_PATH} ]; then
    generate_lcov_report
fi

# 检查gcovr报告文件
if [ -d ${AGENT_TEST_GCOVR_HTML_DIR} ]; then
    rm -rf ${AGENT_TEST_GCOVR_HTML_DIR}
fi
mkdir ${AGENT_TEST_GCOVR_HTML_DIR}

# 使用gcovr生成覆盖率报告
pushd ${AGENT_ROOT}
gcovr -r . --html --html-details \
-f 'src/src/agent/' \
-f 'src/src/alarm/' \
-f 'src/src/apps/appprotect/AppProtectService.cpp' \
-f 'src/src/apps/appprotect/plugininterface/JobServiceHandler.cpp' \
-f 'src/src/apps/appprotect/plugininterface/PluginRegisterHandler.cpp' \
-f 'src/src/apps/appprotect/plugininterface/SecurityServiceHandler.cpp' \
-f 'src/src/apps/appprotect/plugininterface/ShareResourceHandler.cpp' \
-f 'src/src/apps/dws/XBSAClient/DataConversion.cpp' \
-f 'src/src/apps/dws/XBSAClient/File.cpp' \
-f 'src/src/apps/dws/XBSAClient/ThriftClientMgr.cpp' \
-f 'src/src/apps/dws/XBSAClient/xbsa.cpp' \
-f 'src/src/apps/dws/XBSACom/' \
-f 'src/src/apps/dws/XBSAServer/DwsDppClient/' \
-f 'src/src/apps/dws/XBSAServer/BsaDb.cpp' \
-f 'src/src/apps/dws/XBSAServer/BsaIntfAdaptor.cpp' \
-f 'src/src/apps/dws/XBSAServer/BsaMountManager.cpp' \
-f 'src/src/apps/dws/XBSAServer/BSAServiceHandler.cpp' \
-f 'src/src/apps/dws/XBSAServer/BsaSession.cpp' \
-f 'src/src/apps/dws/XBSAServer/BsaSessionManager.cpp' \
-f 'src/src/apps/dws/XBSAServer/BsaTransManager.cpp' \
-f 'src/src/apps/dws/XBSAServer/BsaTransState.cpp' \
-f 'src/src/apps/dws/XBSAServer/CTimer.cpp' \
-f 'src/src/apps/dws/XBSAServer/ThriftServer.cpp' \
-f 'src/src/common/' \
-f 'src/src/host/host.cpp' \
-f 'src/src/message/' \
-f 'src/src/pluginfx/' \
-f 'src/src/plugins/appprotect/' \
-f 'src/src/plugins/dws/' \
-f 'src/src/rootexec/' \
-f 'src/src/securecom/' \
-f 'src/src/servicecenter/' \
-f 'src/src/taskmanager/externaljob/' \
-e 'src/src/common/CMpThread.cpp' \
-e 'src/src/pluginfx/ExternalPluginManager.cpp' \
--exclude-unreachable-branches --exclude-throw-branches -v -o ${GCOVR_HTML_FILE_PATH}
# --exclude-unreachable-branches --exclude-throw-branches -v -o ${GCOVR_HTML_FILE_PATH} > ${LLT_RESULT} 2>&1
popd
find ${AGENT_ROOT} -name "*.gcov" | xargs rm -rf
cat ${LLT_RESULT} | grep "command not found"
if [ $? -eq 0 ]; then
	exit 1
fi
cat ${LLT_RESULT} | grep "ERROR"
if [ $? -eq 0 ]; then
	exit 1
fi

generate_report