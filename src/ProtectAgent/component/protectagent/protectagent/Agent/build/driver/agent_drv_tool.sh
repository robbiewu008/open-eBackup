#!/bin/sh

if [ ${AGENT_ROOT:-0} = 0 ]
then
    echo "please source env.sh first"
    exit 2
fi

CLEAN=0
DRIVER_TOOL_DIR=${AGENT_ROOT}/src/src/tools/driverTools
AGENT_INC_DIR=${AGENT_ROOT}/src/inc
DRIVER_INC_DIR=${AGENT_ROOT}/src/inc/driver/linux
SECUREC_INC_DIR=${AGENT_ROOT}/platform/securec/include 
SECUREC_SRC_DIR=${AGENT_ROOT}/platform/securec/src
DRV_DR_BIN_DIR=${AGENT_ROOT}/bin/
INC_OPEN_TINYXML_DIR=${AGENT_ROOT}/open_src/tinyxml
BASE_OPEN_DIR=${AGENT_ROOT}/open_src
INC_OPEN_CURL_DIR=${AGENT_ROOT}/open_src/curl/include
SHUTDOWN_TOOL_NAME=shutdown2iomirror
STATISTICS_TOOL_NAME=showStatisticsInfo

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
    fi

    
    pushd ${DRIVER_TOOL_DIR}
    
    if [ ${CLEAN} -eq 1 ]
    then
        rm -rf ${SHUTDOWN_TOOL_NAME} ${DRV_DR_BIN_DIR}/${SHUTDOWN_TOOL_NAME}
        rm -rf ${STATISTICS_TOOL_NAME}  ${DRV_DR_BIN_DIR}/${STATISTICS_TOOL_NAME}
        rm -rf ${DRV_DR_BIN_DIR}/*.o
    else
        g++ shutdown2iomirror.cpp ${SECUREC_SRC_DIR}/memset_s.c -o ${SHUTDOWN_TOOL_NAME} -I${AGENT_INC_DIR} -I${DRIVER_INC_DIR} -I${SECUREC_INC_DIR} 
        g++ showStatisticsInfo.cpp -o ${STATISTICS_TOOL_NAME} -I${AGENT_INC_DIR} -I${BASE_OPEN_DIR} -I${SECUREC_INC_DIR} -I${INC_OPEN_TINYXML_DIR} -I${INC_OPEN_CURL_DIR} -L${DRV_DR_BIN_DIR} -ldl -lrt -lcommon
        # delete the old tools located in Agent/bin directory
        [ -f "${DRV_DR_BIN_DIR}/${SHUTDOWN_TOOL_NAME}" ] && rm -rf ${DRV_DR_BIN_DIR}/${SHUTDOWN_TOOL_NAME}
        [ -f "${DRV_DR_BIN_DIR}/${STATISTICS_TOOL_NAME}" ] &&  rm -rf ${DRV_DR_BIN_DIR}/${STATISTICS_TOOL_NAME}
  
        [ -f "${SHUTDOWN_TOOL_NAME}" ] && echo "  -- ${SHUTDOWN_TOOL_NAME} tool compiler succ."
        [ -f "${STATISTICS_TOOL_NAME}" ] && echo "  -- ${STATISTICS_TOOL_NAME} tool compiler succ."

        cp -r ${SHUTDOWN_TOOL_NAME} ${DRV_DR_BIN_DIR}/${SHUTDOWN_TOOL_NAME}
        cp -r ${STATISTICS_TOOL_NAME} ${DRV_DR_BIN_DIR}/${STATISTICS_TOOL_NAME}
    fi
    
    popd
}

echo "#########################################################"
echo "   Copyright (C), 2013-2014, Huawei Tech. Co., Ltd."
echo "   Start to compile Agent driver tools"
echo "#########################################################"
StartTime=`date '+%Y-%m-%d %H:%M:%S'`

make_init $*

main_enter

EndTime=`date '+%Y-%m-%d %H:%M:%S'`
echo "#########################################################"
echo "   Compile Agent driver tools completed."
echo "   begin at ${StartTime}"
echo "   end   at ${EndTime}"
echo "#########################################################"
