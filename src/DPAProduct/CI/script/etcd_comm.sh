#!/bin/bash
########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################
L_BRANCH=$1
G_ETCD_URL="ebkcompile.hic.cloud:2379"

G_RET_SUCCESS=0
G_RET_FAILED=1

#success
G_STR_SUCCESS="c3VjY2Vzcw=="
#failed
G_STR_COMPILE_ERR="ZmFpbGVk"

set_keys()
{
    if [ $# -ne 2 ];then
        echo "set_keys No keys found"
        return ${G_RET_FAILED}        
    fi    
    
    local L_KEY=$1
    local L_VALUE=$2
    
    local L_BASE64_KEY=$(printf "${L_KEY}" | base64)    
    local L_RET=`curl -L http://${G_ETCD_URL}/v3/kv/put -X POST -d "{\"key\": \"${L_BASE64_KEY}\", \"value\": \"${L_VALUE}\"}"`
    if [ $? -ne 0 ];then
        echo "set_keys curl failed"
        return ${G_RET_FAILED}
    fi
    
    echo ${L_RET} | grep error
    if [ $? -eq 0 ];then
        echo "set_keys etcd failed"
        return ${G_RET_FAILED}
    fi
    
    return ${G_RET_SUCCESS}
}

clear_keys()
{
    local L_KEY=$1
    if [ -z ${L_KEY} ];then
        echo "clear_keys No key"
        return ${G_RET_FAILED}
    fi
    
    local L_BASE64_KEY=$(printf "${L_KEY}" | base64)
    local L_RET=`curl -L http://${G_ETCD_URL}/v3/kv/deleterange -X POST -d "{\"key\": \"${L_BASE64_KEY}\", \"range_end\": \"XDA=\"}"`
    if [ $? -ne 0 ];then
        echo "clear_keys curl failed"
        return ${G_RET_FAILED}
    fi
    
    echo ${L_RET} | grep error
    if [ $? -eq 0 ];then
        echo "clear_keys etcd failed"
        return ${G_RET_FAILED}
    fi
    
    return ${G_RET_SUCCESS}    
}

wait_keys_success()
{
    local L_KEY=$1
    if [ -z ${L_KEY} ];then
        echo "wait_keys_success No key"
        return ${G_RET_FAILED}
    fi    
    
    local L_BASE64_KEY=$(printf "${L_KEY}" | base64)
    local L_ms_make_time=360    
    for((i = 0; i <= ${L_ms_make_time}; i++))
    do
        
        local L_RET=`curl -L http://${G_ETCD_URL}/v3/kv/range -X POST -d "{\"key\": \"${L_BASE64_KEY}\"}"`
        if [ $? -ne 0 ];then
            echo "wait_keys_success curl failed, wait 10s."
            sleep 10
            continue
        fi        
        
        echo ${L_RET} | grep error
        if [ $? -eq 0 ];then
            echo "wait_keys_success etcd failed, wait 10s."
            sleep 10
            continue
        fi
        
        echo ${L_RET} | grep ${G_STR_SUCCESS}
        if [ $? -eq 0 ];then
            echo "wait_keys_success ${L_KEY} success"
            return ${G_RET_SUCCESS}
        fi

        echo ${L_RET} | grep ${G_STR_COMPILE_ERR}
        if [ $? -eq 0 ];then
            echo "wait_keys_success ${L_KEY} error"
            return ${G_RET_FAILED}
        fi    
            
        echo "Package on ${L_KEY} is not complete, wait 10s."
        sleep 10
        continue        
    done
    
    echo "Package on ${L_KEY} is timeout." 
    return ${G_RET_FAILED}
}

start_compile()
{
    local L_BRANCH=$1
    echo "start_compile on ${L_BRANCH}"
    
    if [ -z ${L_BRANCH} ];then
        echo "start_compile No branch"
        return ${G_RET_FAILED}
    fi
    
    for((i = 0; i <= 3; i++))
    do
        clear_keys "/${L_BRANCH}"
        if [ $? -eq 0 ];then
            return ${G_RET_SUCCESS} 
        fi
        
        echo "start_compile on ${L_BRANCH} error, retry in 10s"
        sleep 10
        continue      
    done
    
    echo "start_compile error"
    return ${G_RET_FAILED}  
}

wait_branch_component_success()
{
    local L_BRANCH=$1
    local L_COMPONENT=$2
    
    if [ -z ${L_BRANCH} ];then
        echo "wait_branch_component_success No branch"
        return ${G_RET_FAILED}
    fi

    if [ -z ${L_COMPONENT} ];then
        echo "wait_branch_component_success No component"
        return ${G_RET_FAILED}
    fi
    
    local L_KEY="/${L_BRANCH}/${L_COMPONENT}"
    wait_keys_success ${L_KEY}
    if [ $? -ne 0 ];then
        echo "wait_branch_component_success failed on ${L_KEY}"
        return ${G_RET_FAILED}
    fi
    
    return ${G_RET_SUCCESS}
}

set_branch_component_result()
{
    local L_BRANCH=$1
    local L_COMPONENT=$2
    local L_RESULT=$3
    
    if [ -z ${L_BRANCH} ];then
        echo "set_branch_component_success No branch"
        return ${G_RET_FAILED}
    fi

    if [ -z ${L_COMPONENT} ];then
        echo "set_branch_component_success No component"
        return ${G_RET_FAILED}
    fi
    
    if [ -z ${L_RESULT} ];then
        echo "set_branch_component_success No result"
        return ${G_RET_FAILED}
    fi
    
    local L_STR=""
    if [ ${L_RESULT} -eq 0 ];then
        L_STR=${G_STR_SUCCESS}
    else
        L_STR=${G_STR_COMPILE_ERR}
    fi
    
    local L_KEY="/${L_BRANCH}/${L_COMPONENT}"
    
    for((i = 0; i <= 3; i++))
    do
        set_keys ${L_KEY} ${L_STR}
        if [ $? -eq 0 ];then
            return ${G_RET_SUCCESS}
        fi
        
        echo "set_branch_component_success on ${L_BRANCH} error, retry in 10s"
        sleep 10
        continue      
    done
    
    echo "set_branch_component_success error"
    return ${G_RET_FAILED}  
}

get_env_type()
{
    if [ -f /etc/euleros-release ];then
        local arch_type=`uname -m`
        if [ "$arch_type" == "aarch64" ];then
            PKG_TYPE="euler-arm"
        else
            PKG_TYPE="euler-x86"
        fi    
    else
        PKG_TYPE="suse-x86"
    fi
}

get_env_type
