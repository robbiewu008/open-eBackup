#!/bin/bash
######################################################################
## @Company: HUAWEI Tech. Co., Ltd.
## @Filename send_alarm.sh
## @Usage ./send_alarm.sh event_id event_type event_cause event_description
##                    addition_info hostname ha_name peer_hostname
##                    peer_haname
## @Description 发生主备倒换等时间时，调用该脚本通知相关模块做处理
## @Created 2014.08.21
#####################################################################

#####################################################################
## 事件参数：
## event_id         事件ID（0：心跳中断；1：数据同步异常；2：主备倒换
##                          3：单机重启；4：链路中断；5：网络中断 6：网关不通）
## event_type       事件类型（0:故障告警；1:恢复告警；2:事件））
## event_cause      事件产生原因
## event_description事件描述
## addition_info    事件的额外信息
## hostname         本节点的主机名
## ha_name          本节点的HA名称
## peer_hostname    对端的主机名
## peer_haname      对端HA的名称
#####################################################################
G_HA_SCRIPT_PATH=/usr/local/ha/script
source $G_HA_SCRIPT_PATH/event_lib.sh
G_RESOURCE_NAME="HA"
declare event_id=$1
declare event_type=$2
declare event_cause=$3
declare event_desc=$4
declare addition_info=$5
declare local_name=$6
declare ha_name=$7
declare peer_name=$8
declare peer_ha_name=$9

# 脚本的入参个数
declare -i PARAM_NUM=9

declare script_name=`basename $0`

#####################################################################
## @Usage log
## @Return 0
## @Description 记录到日志
#####################################################################
log()
{
    local log_level="$1" && shift
    echo `date` "${log_level}:" "$*($$)"
}

#####################################################################
## @Usage check_param $@
## @Return 0
## @Description 检查参数是否合法
#####################################################################
function check_param()
{
    if [ $# -ne $PARAM_NUM ]; then
        log "ERROR" "param num is not corrent"
        return 1
    fi
    return 0
}

#####################################################################
## @Usage process_event
## @Return 0
## @Description 根据事件ID，处理相应的事件
#####################################################################
function process_event()
{
    case $event_id in
        0)
            # 处理心跳中断告警
            if [ $event_type == 0 ];then
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "create EVENT_HA_HEARTBEAT_LOST alarm"
            process_ha_alarm "create" "EVENT_HA_HEARTBEAT_LOST" $G_RESOURCE_NAME
            else
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "recover EVENT_HA_HEARTBEAT_LOST alarm"
            process_ha_alarm "recover" "EVENT_HA_HEARTBEAT_LOST" $G_RESOURCE_NAME
            fi
            exit 0
            ;;
        1)
            # 处理数据同步失败告警
            if [ $event_type == 0 ];then
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "create EVENT_HA_FILE_SYNC_FAILED alarm"
            process_ha_alarm "create" "EVENT_HA_FILE_SYNC_FAILED" $G_RESOURCE_NAME
            else
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "recover EVENT_HA_FILE_SYNC_FAILED alarm"
            process_ha_alarm "recover" "EVENT_HA_FILE_SYNC_FAILED" $G_RESOURCE_NAME
            fi
            exit 0
            ;;
        2)
            # 处理主备倒换事件
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "create EVENT_HA_SWITCH event"
            process_ha_alarm "event" "EVENT_HA_SWITCH" $G_RESOURCE_NAME
            exit 0
            ;;
        3)
            # 处理单机重启事件
            exit 0
            ;;
        4)
            # 处理链路中断告警
            exit 0
            ;;
        5)
            # 处理网络中断事件
            exit 0
            ;;
		    6)
            # 处理网关不通告警
            if [ $event_type == 0 ];then
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "create EVENT_HA_GATEWAY_CANNOT_REACH alarm"
            process_ha_alarm "create" "EVENT_HA_GATEWAY_CANNOT_REACH" $G_RESOURCE_NAME
            else
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "recover EVENT_HA_GATEWAY_CANNOT_REACH alarm"
            process_ha_alarm "recover" "EVENT_HA_GATEWAY_CANNOT_REACH" $G_RESOURCE_NAME
            fi
            exit 0
            ;;
        *)
            exit 1
            ;;
    esac
}

function main()
{
    check_param "$@" || { exit 1; }
    process_event "$@"; ret=$?
    return $ret
}

log "begin execute $script_name"
main "$@";ret=$?
log "finish execute $script_name, result is $ret"
exit $ret