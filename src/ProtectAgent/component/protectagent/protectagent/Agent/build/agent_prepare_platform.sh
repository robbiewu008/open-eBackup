#!/bin/sh

sys=`uname -s`
if [ "$sys" = "SunOS" ]; then
    AWK=nawk
else
    AWK=awk
fi

CURRENT_DIR=$(cd "$(dirname $0)" && pwd)

AGENT_ROOT=$CURRENT_DIR/..
PLATFORM_DIR=$AGENT_ROOT/platform
PLATFORM_SECUREC_DIR=$PLATFORM_DIR/securec
PLATFORM_KMC_DIR=$PLATFORM_DIR/kmc

THRIFT_BIN_DIR=${AGENT_ROOT}/open_src/thrift/.libs/bin

PrepareThriftTool()
{
    if [ ! -d "${THRIFT_BIN_DIR}" ]; then
        echo "Thrift bin path not exists."
        exit 1
    fi

    if [ ! -f "${THRIFT_BIN_DIR}/thrift" ]; then
        echo "Thrift binary not exists"
        exit 1
    fi

    chmod 777 ${THRIFT_BIN_DIR}/thrift
}

# Agent框架thrift
GenerateXbsaThriftCPP()
{
    cd ${THRIFT_BIN_DIR}
    cp -f ${AGENT_ROOT}/src/src/apps/dws/XBSAServer/xbsa.thrift .
    DEFAULT_DIR=gen-cpp
    if [ -d "${DEFAULT_DIR}" ]; then
        rm -rf ${DEFAULT_DIR}
    fi

    ./thrift -r -gen cpp xbsa.thrift >> ./build_make.log 2>&1
    main_result=$?
    if [ ${main_result} -ne 0 ]; then
        echo "Generate xbsa thrift cpp failed."
        exit ${main_result}
    fi

    if [ ! -d "${DEFAULT_DIR}" ]; then
        echo "Generate thrift cpp failed."
        exit 1
    fi

    # 将skeleton.cpp改名为skeleton.cpp.bak，不拷贝到源代码目录
    FILENAME=`ls ${DEFAULT_DIR}/*skeleton.cpp`
    NEW_FILE_NAME=$(echo ${DEFAULT_DIR}/$(basename ${FILENAME}).bak)
    mv  ${FILENAME} ${NEW_FILE_NAME}
    # 拷贝到server端
    cp -f ${DEFAULT_DIR}/*.h  ${AGENT_ROOT}/src/inc/apps/dws/XBSAServer
    cp -f ${DEFAULT_DIR}/*.cpp ${AGENT_ROOT}/src/src/apps/dws/XBSAServer
    # 拷贝到公共
    cp -f ${DEFAULT_DIR}/*.h  ${AGENT_ROOT}/src/inc/xbsaclientcomm
    cp -f ${DEFAULT_DIR}/*.cpp ${AGENT_ROOT}/src/src/apps/xbsaclientcomm
}


# Agent框架thrift
GenerateFrameThriftCPP()
{
    echo "generate frame thrift code"

    DEFAULT_DIR=gen-cpp
    THRIFT_SERVICE_DIR=${AGENT_ROOT}/src/src/servicecenter/thrift
    THRIFT_SERVICE_OUTPUT=${AGENT_ROOT}/src/src/servicecenter/thrift/${DEFAULT_DIR}
    rm -rf ${THRIFT_SERVICE_OUTPUT}

    ${THRIFT_BIN_DIR}/thrift -o ${THRIFT_SERVICE_DIR} -r -gen cpp ${THRIFT_SERVICE_DIR}/ApplicationProtectBaseDataType.thrift
    main_result=$?
    if [ ${main_result} -ne 0 ];then
        echo "ApplicationProtectBaseDataType.thrift gen fail, ${main_result}"
        exit ${main_result}
    fi

    ${THRIFT_BIN_DIR}/thrift -o ${THRIFT_SERVICE_DIR} -r -gen cpp ${THRIFT_SERVICE_DIR}/ApplicationProtectFramework.thrift
    main_result=$?
    if [ ${main_result} -ne 0 ];then
        echo "ApplicationProtectFramework.thrift gen fail"
        exit ${main_result}
    fi

    ${THRIFT_BIN_DIR}/thrift -o ${THRIFT_SERVICE_DIR} -r -gen cpp ${THRIFT_SERVICE_DIR}/ApplicationProtectPlugin.thrift
    main_result=$?
    if [ ${main_result} -ne 0 ];then
        echo "ApplicationProtectPlugin.thrift gen fail"
        exit ${main_result}
    fi

    FILENAMES=`ls ${THRIFT_SERVICE_OUTPUT}/*skeleton.cpp`
    for FILENAME in $FILENAMES;
    do
        rm -f "${FILENAME}"
    done

    # 拷贝到server端
    cp -f ${THRIFT_SERVICE_OUTPUT}/*.h  ${AGENT_ROOT}/src/inc/apps/appprotect/plugininterface
    cp -f ${THRIFT_SERVICE_OUTPUT}/*.cpp ${AGENT_ROOT}/src/src/apps/appprotect/plugininterface

    echo "#########################################################"
    echo "   generate frame thrift cpp succ."
    echo "#########################################################"
}

# $1: pkg path
# $2: pkg unzip dir
# $3: target dir
# $4: name
PreparePlatform()
{
    PKG_PATH=$1
    PKG_DIR_NAME=$2
    TARGET_DIR=$3
    MODULE_NAME=$4

    if [ ! -f $PKG_PATH ]; then
        echo "$MODULE_NAME package not found: $PKG_PATH"
        return 1
    fi

    if [ ! -d "$TARGET_DIR" ]; then
        mkdir $TARGET_DIR
    fi

    if [ -d "$PLATFORM_DIR/$PKG_DIR_NAME" ]; then
        rm -rf $PLATFORM_DIR/$PKG_DIR_NAME/*
    fi

    cd $PLATFORM_DIR

    tar -zxf $PKG_PATH

    cp -rf $PLATFORM_DIR/$PKG_DIR_NAME/* $TARGET_DIR

    rm -rf $PLATFORM_DIR/$PKG_DIR_NAME

    echo "Prepare platform $MODULE_NAME success."
}

main()
{
    if [ ! -d "$PLATFORM_DIR" ]; then
        echo "No platform dir $PLATFORM_DIR."
        return 1
    fi

    PreparePlatform "$PLATFORM_DIR/SecureCLib_rel.tar.gz" "SecureCLib_rel" "$PLATFORM_SECUREC_DIR" "recurec"
    if [ $? != 0 ]; then
        echo "Prepare securec fail."
        return 1
    fi

    PreparePlatform "$PLATFORM_DIR/KMCV3_rel.tar.gz" "KMCV3_rel" "$PLATFORM_KMC_DIR" "kmc"
    if [ $? != 0 ]; then
        echo "Prepare kmc fail."
        return 1
    fi

    cp -f $PLATFORM_SECUREC_DIR/lib/libsecurec.a $PLATFORM_KMC_DIR/lib/

    PrepareThriftTool

    GenerateXbsaThriftCPP

    GenerateFrameThriftCPP 
}

main $*
