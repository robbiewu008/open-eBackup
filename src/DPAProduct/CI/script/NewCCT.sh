#!bin/bash

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
WORKSPACE=${CUR_PATH}/../../..

START_TIME=$1
END_TIME=$2

if [ -z ${START_TIME} ];then
	echo "参数缺失：开始统计时间"
	exit 1
fi

if [ -z ${END_TIME} ];then
	echo "参数缺失：开始统计时间+1"
	exit 1
fi

code_xml="${WORKSPACE}/DPAProduct/.cloudbuild/euler_arm/code.xml"

#删除DataEnableEngine/component/PM_App_Common_Lib
sed -i '48d' ${code_xml}

cd ${WORKSPACE}/

function down_code() {

    L_DIR=$1
    mkdir -p ${WORKSPACE}/${L_DIR}
    cat ${code_xml} | grep "project" > list.txt

    while read line
    do
        cd ${WORKSPACE}/${L_DIR}
        echo ${line}
        REPO_TYPE=$(echo ${line} | awk -F 'remote="' '{print $2}' | awk -F '\"' '{print $1}')
        if [ "${REPO_TYPE}" == "codehub" ]; then
                REPO_URL="ssh://git@codehub-dg-y.huawei.com:2222/"$(echo ${line} | awk -F 'name="' '{print $2}' | awk -F '\"' '{print $1}')
        elif [ "${REPO_TYPE}" == "gitlab" ]; then
                REPO_URL="ssh://git@gitlab.huawei.com:2222/"$(echo ${line} | awk -F 'name="' '{print $2}' | awk -F '\"' '{print $1}')
        else
                echo REPO_TYPE=${REPO_TYPE}
                echo "repo type error"
                exit 1
        fi

        REPO_PATH=$(echo ${line} | awk -F 'path="' '{print $2}' | awk -F '\"' '{print $1}')
        echo "start download ${REPO_URL} to ${REPO_PATH}"
        git clone ${REPO_URL} ${REPO_PATH}
        cd - > /dev/null
    done  < list.txt

    mv ${WORKSPACE}/${L_DIR}/DataEnableEngine/component/*  ${WORKSPACE}/${L_DIR}/
    mv ${WORKSPACE}/${L_DIR}/DataMoveEngine/component/*  ${WORKSPACE}/${L_DIR}/
    mv ${WORKSPACE}/${L_DIR}/ProtectManager/component/*  ${WORKSPACE}/${L_DIR}/
    mv ${WORKSPACE}/${L_DIR}/Infrastructure_OM/*  ${WORKSPACE}/${L_DIR}/
    rm -rf  ${WORKSPACE}/${L_DIR}/fit
    rm -rf  ${WORKSPACE}/${L_DIR}/Infrastructure_OM

}

function checkout() {
        
    DIR="${WORKSPACE}/Base/$1"
    cd ${DIR}
    pwd
    git log --since=${START_TIME} --until=${END_TIME} > commit
    commitid=`head -n +1 commit | awk '{print $2}'`
    if [ ! -s "commit" ]; then
        git log | grep "commit " > commit
        commitid=$(tail -n 1 commit | awk -F ' ' '{print $2}')
        if [ ${#commitid} -ne 40 ]; then
            commitid=$(tail -n 2 commit | head -n 1 | awk -F ' ' '{print $2}')
        fi
    fi

    git checkout ${commitid}
    cd ${WORKSPACE}/Base

}

function base_checkout() {
    
    DIR=$(ls ${WORKSPACE}/Base)
    for ms in ${DIR}; do
        if [ -d ${ms} ]; then
            echo "start checkout ${ms}"
            checkout ${ms}
            if [ $? -ne 0 ]; then
                echo "${ms} checkout failed!"
            fi
        fi
    done

}

function main() {

    down_code Base
    if [ $? -ne 0 ]; then
        echo "download Base code error!"
        exit 1
    fi

    down_code Target
    if [ $? -ne 0 ]; then
        echo "download Target code error!"
        exit 1
    fi
    
    base_checkout
    if [ $? -ne 0 ]; then
        echo "checkout error!"
        exit 1
    fi
    
}

main
if [ $? -ne 0 ]; then
    exit 1
fi
