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

set +x
G_HA_INSTALL_PATH=/usr/local/ha
G_HA_SCRIPT_PATH=/usr/local/ha/script
G_SUDO_SCRIPT_PATH=/opt/script
source ${G_HA_SCRIPT_PATH}/log.sh
source ${G_HA_SCRIPT_PATH}/event_lib.sh

G_RESOURCE_NAME="floatIp"
G_CONF_HA_SCRIPT_PATH=${G_HA_INSTALL_PATH}/module/hacom/script
G_HAARB_XML=${G_HA_INSTALL_PATH}/module/haarb/conf/haarb.xml
G_IFCONFIG="$(which ifconfig)"
G_IP="$(which ip)"
G_HA_MODE=""
G_ARPING="$(which arping)"
G_NDSEND=""

#G_FLOAT_IP_BAK=$G_HA_INSTALL_PATH/conf/floatIpBak
G_MANAGE_FLOAT_IP=""
G_MANAGE_FIP_PORT=""
G_MANAGE_FIP_MASK=""
G_MANAGEMENT_ETH_NAME=""
IP_FLAG=""
G_MANAGEMENT_IP=""
G_PAIR_LOCAL_IP=""
G_FLOAT_IP_NAME_TAIL=0


#******************************************************************#
# Function: set_floatip
# Description: The function to set floatIP
# Input Parameters:
# None
# Return : 0 success
#      1 failed
#******************************************************************#
function set_floatip()
{
    if [ $IP_FLAG = 'ipv4' ];then
        log_info "[${FUNCNAME[0]}(),$LINENO] begin set_floatIP,floatPort:${G_MANAGE_FIP_PORT},floatIP:${G_MANAGE_FLOAT_IP},mask:${G_MANAGE_FIP_MASK}"
        ping -c 3 ${G_MANAGE_FLOAT_IP} >/dev/null
        if [ $? -ne 0 ];then
            sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh set_floatip ${G_MANAGE_FIP_PORT} ${G_MANAGE_FLOAT_IP} ${G_MANAGE_FIP_MASK} ${G_PAIR_LOCAL_IP_RT_ID}
            if [ $? -ne 0 ];then
                log_error "[${FUNCNAME[0]}(),$LINENO] set float ip  ${G_MANAGE_FLOAT_IP} failed !"
                return 1
            fi
        else
            log_warn "[${FUNCNAME[0]}(),$LINENO] ${G_MANAGE_FLOAT_IP} has been used somewhere, please check whether if the floatIP is available "
        fi
        log_info "[${FUNCNAME[0]}(),$LINENO] set_floatIP success !"
        return 0
    else
        log_info "[${FUNCNAME[0]}(),$LINENO] begin set_floatIP,floatPort:${G_MANAGE_FIP_PORT},floatIP:${G_MANAGE_FLOAT_IP},mask:${G_MANAGE_FIP_MASK}"
        ping6 -c 3 ${G_MANAGE_FLOAT_IP} >/dev/null
        if [ $? -ne 0 ];then
            sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh set_floatip ${G_MANAGE_FIP_PORT} ${G_MANAGE_FLOAT_IP} ${G_MANAGE_FIP_MASK} ${G_PAIR_LOCAL_IP_RT_ID}
            if [ $? -ne 0 ];then
                log_error "[${FUNCNAME[0]}(),$LINENO] set float ip  ${G_MANAGE_FLOAT_IP} failed !"
                return 1
            fi
        else
            log_warn "[${FUNCNAME[0]}(),$LINENO]  ${G_MANAGE_FLOAT_IP} has been used somewhere, please check whether if the floatIP is available "
        fi

        log_info "[${FUNCNAME[0]}(),$LINENO] set_floatIP success! ndsend: ${G_NDSEND} ${G_MANAGE_FIP_PORT} ${G_MANAGE_FLOAT_IP}"
        return 0
    fi
}


#******************************************************************#
# Function: repair
# Description: The function to be called when host abnormal
# Input Parameters:
# None
# Return : 0 success
#      1 failed
#******************************************************************#
function repair()
{
    log_info "[${FUNCNAME[0]}(),$LINENO] begin repair !"
    get_floatip_information
    if [ $? -ne 0 ];then
        log_error "[${FUNCNAME[0]}(),$LINENO] get_floatip_information failed !"
        return 1
    fi
    set_floatip
    return $?
}

#******************************************************************#
# Function: status
# Description: The function to query the status of floatIP
# Input Parameters:
# None
# Return : 0 running
#      2 stopped
#******************************************************************#
function status()
{
    if [ $IP_FLAG = 'ipv4' ];then
        log_info "[${FUNCNAME[0]}(),$LINENO] begin query floatIP status !"

        ${G_IP} addr 2> /dev/null | grep "${G_MANAGE_FLOAT_IP}" > /dev/null
        if [ $? -eq 0 ];then
            log_info "[${FUNCNAME[0]}(),$LINENO] floatIP has been set !"
            return 0
        fi
        log_info "[${FUNCNAME[0]}(),$LINENO] floatIP has been not set !"
        return 2
    else
        log_info "[${FUNCNAME[0]}(),$LINENO] begin query floatIP status !"

        ${G_IP} addr 2> /dev/null | grep "${G_MANAGE_FLOAT_IP}" > /dev/null
        if [ $? -eq 0 ];then
            log_info "[${FUNCNAME[0]}(),$LINENO] floatIP has been set !"
            return 0
        fi
        log_info "[${FUNCNAME[0]}(),$LINENO] floatIP has not been set !"
        return 2
    fi
}

#******************************************************************#
# Function: get_pair_local_ip
# Description: The function to get the internal communication network ip
# Input Parameters:
# None
# Return : 0 success
#          1 failed
#******************************************************************#
function get_pair_local_ip()
{
    pair_ip=`get_pair_ip`
    if [ "${pair_ip}" == ${G_UNDEFINED_STRING} ]; then
        log_error "[${FUNCNAME[0]}(),$LINENO] do not find pair local ip."
        return 1
    fi
    for ip in $pair_ip;do
        ip a  | grep $ip
        if [ $? -eq 0 ]; then
            G_PAIR_LOCAL_IP=${ip}
            G_PAIR_LOCAL_IP_RT_ID=`ip rule | grep "from ${ip} lookup" |  awk -F " " {'print $5'}`
            log_info "[${FUNCNAME[0]}(),$LINENO] pair local ip is ${ip}. rt is ${G_PAIR_LOCAL_IP_RT_ID}!"
            return 0
        fi
    done
    log_error "[${FUNCNAME[0]}(),$LINENO] do not find internal communication ip."
    return 1
}

#******************************************************************#
# Function: get_floatip_information
# Description: The function to get the floatip's information
# Input Parameters:
# None
# Return : 0 success
#          1 failed
#******************************************************************#
function get_floatip_information()
{
    if [ $IP_FLAG = 'ipv4' ];then
        G_MANAGE_FLOAT_IP=`get_float_ip`
        if [ -z $G_MANAGE_FLOAT_IP ];then
            log_error "[${FUNCNAME[0]}(),$LINENO] get floatIP address faild"
            return 1
        fi

        log_info "[${FUNCNAME[0]}(),$LINENO] floatip=$G_MANAGE_FLOAT_IP"
        G_MANAGE_FIP_MASK="`${G_IP} addr 2> /dev/null | grep $G_PAIR_LOCAL_IP | awk '{print $2}' | awk -F '/' '{print $2}'`"

        if [ -z $G_MANAGE_FIP_MASK ];then
            log_error "[${FUNCNAME[0]}(),$LINENO] find mask of managementIP faild"
            return 1
        fi

        local gs_pair_eth_name=""
        gs_pair_eth_name=`${G_IP} a 2> /dev/null | grep -n $G_PAIR_LOCAL_IP | awk '{print $NF}' |cut -d ":" -f 1`
        if [ -z $gs_pair_eth_name ];then
            log_error "[${FUNCNAME[0]}(),$LINENO] find device_name of managementIP faild"
            return 1
        fi

        G_MANAGE_FIP_PORT=$gs_pair_eth_name
        log_info "[${FUNCNAME[0]}(),$LINENO] get floatIP info success,floatIP_eth_name=$G_MANAGE_FIP_PORT"
        return 0
    else
        G_MANAGE_FLOAT_IP=`get_float_ip`
        if [ -z $G_MANAGE_FLOAT_IP ];then
            log_error "[${FUNCNAME[0]}(),$LINENO] get floatIP address faild"
            return 1
        fi

        log_info "[${FUNCNAME[0]}(),$LINENO] floatip=$G_MANAGE_FLOAT_IP"

        local gs_pair_eth_name=""
        gs_pair_eth_name=`${G_IP} addr 2> /dev/null | grep -n $G_PAIR_LOCAL_IP | awk '{print $NF}' |cut -d ":" -f 1`
        if [ -z $lin ];then
            log_error "[${FUNCNAME[0]}(),$LINENO] find device_name line number of managementIP faild"
            return 1
        fi

        G_MANAGE_FIP_PORT=$gs_pair_eth_name
        log_info "[${FUNCNAME[0]}(),$LINENO] get floatIP info success,floatIP_eth_name=$G_MANAGE_FIP_PORT"
        return 0
    fi
}

#******************************************************************#
# Function: check_hamode
# Description: The function to check mode of HA
# Input Parameters:
# None
# Return : 0 double mode
# exit : 0 single mode
#******************************************************************#
function check_hamode()
{
    G_HA_MODE=`get_ha_mode`
    if [ "X${G_HA_MODE}" == "Xsingle" ];then
        log_info "[${FUNCNAME[0]}(),$LINENO] this is single mode"
        return 1
    else
        log_info "[${FUNCNAME[0]}(),$LINENO] this is double mode"
        return 0
    fi

}

function check_ip_flag()
{
    if [[ $G_MANAGE_FLOAT_IP =~ .*:.* ]]; then
        IP_FLAG="ipv6"
        G_NDSEND="$(which ndsend)"
    else
        IP_FLAG="ipv4"
    fi
}

function check_ip_conflicts() {
    local ip=$1
    local res=$($G_ARPING -I $G_MANAGE_FIP_PORT -D -c 1 $ip | grep "Unicast reply")

    if [ -z "$res" ]; then
        log_info "[${FUNCNAME[0]}(),$LINENO] not find conflict ip, res is $res."
        return 0
    else
        local mac_address=$(echo ${res} | grep -o  "[a-f0-9A-F]\\([a-f0-9A-F]\\:[a-f0-9A-F]\\)\\{5\\}[a-f0-9A-F]")
        log_error "[${FUNCNAME[0]}(),$LINENO] ip $ip conflicts, mac address is $mac_address."
        return 1
    fi
    return 0
}

#******************************************************************#
# Function: main
# Description: main
# Input Parameters:action
# Return : 0 double mode
#exit code:
# single / active-active
#   0 = normal
#   1 = abnormal
#   2 = stop
#   3 = unknown
#   4 = starting
#   5 = stopping
#   10 = invaild action
#
# active-standby
#   1 = abnormal
#   2 = stop
#   3 = unknown
#   6 = active normal
#   7 = standby normal
#   8 = switching to active
#   9 = switching to standby
#   10 = invaild action
#******************************************************************#
function main ()
{
    G_MANAGE_FLOAT_IP=`get_float_ip`
    if [ "$G_MANAGE_FLOAT_IP" == "$G_UNDEFINED_STRING" ];then
        log_error "[${FUNCNAME[0]}(),$LINENO] get floatIP address failed."
        exit 1
    fi
    check_ip_flag
    check_hamode
    get_pair_local_ip
    if [ $? -ne 0 ];then
        log_error "[${FUNCNAME[0]}(),$LINENO] get internal ip address failed."
        exit 1
    fi

    case $1 in
    status)
        sleep $[ ( $RANDOM % 3 ) ]s
        get_floatip_information
        if [ $? -ne 0 ];then
            exit 2
        fi
        # 检查浮动ip是否冲突
        if [ "$G_ROLE" == "active" ]; then
            check_ip_conflicts $G_MANAGE_FLOAT_IP
            if [ $? -ne 0 ]; then
                process_ha_alarm "create" "EVENT_HA_FLOAT_IP_CONFLICT" $G_RESOURCE_NAME
            else
                process_ha_alarm "recover" "EVENT_HA_FLOAT_IP_CONFLICT" $G_RESOURCE_NAME
            fi
        fi
        status
        exit $?
        ;;
    repair)
        repair
        if [ $? -ne 0 ];then
            exit 1
        fi
        ;;
    start)
        get_floatip_information
        if [ $? -ne 0 ];then
            exit 1
        fi
        set_floatip
        exit $?
        ;;
    stop)
        get_floatip_information
        sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh unset_floatip ${G_MANAGE_FIP_PORT} ${G_MANAGE_FLOAT_IP} ${G_MANAGE_FIP_MASK} ${G_PAIR_LOCAL_IP_RT_ID}
        exit 0
        ;;
    force-stop)
        get_floatip_information
        sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh unset_floatip ${G_MANAGE_FIP_PORT} ${G_MANAGE_FLOAT_IP} ${G_MANAGE_FIP_MASK} ${G_PAIR_LOCAL_IP_RT_ID}
        exit 0
        ;;
    notify)
        local event_type=$4
        if [ $event_type == 1 ];then
            ##$1: type, e.g.: create, recover, event
            ##$2: event name, e.g.: EVENT_HA_STATUS_EXCEPTION
            ##$3: resource, e.g.: gaussdb, floatIp...
            log_info "[${FUNCNAME[0]}(),$LINENO] create alarm EVENT_HA_STATUS_EXCEPTION,resource:$G_RESOURCE_NAME"
        else
            log_info "[${FUNCNAME[0]}(),$LINENO] recover alarm EVENT_HA_STATUS_EXCEPTION,resource:$G_RESOURCE_NAME"
        fi

        exit $?
        ;;
    diagnose)
        if [ "$G_ROLE" == "primary" ]; then
            sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_clearrmfault ${G_RESOURCE_NAME}
            if [ $? -ne 0 ];then
                log_error "[${FUNCNAME[0]}(),$LINENO] ha_client_tool --clearrmfault --name=${G_RESOURCE_NAME} failed!"
                exit 1
            fi
            log_info "[${FUNCNAME[0]}(),$LINENO] ha_client_tool --clearrmfault --name=${G_RESOURCE_NAME} success!"
        fi
        exit 0
        ;;
    *)
        echo "Parameter incorrect."
        exit 1
        ;;
    esac
}

# 获取脚本入参
in_param_num="$#"
in_param_lst="$@"
declare script_name="$(basename $0)"
log_warn "[${FUNCNAME[0]}(),$LINENO] enter the script ${script_name}($in_param_num): $0 $in_param_lst"

G_ROLE=$2
main "$@"