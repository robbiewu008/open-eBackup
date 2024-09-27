#!/bin/sh
#!/bin/sh

G_HA_INSTALL_PATH=/usr/local/ha
G_HA_SCRIPT_PATH=/usr/local/ha/script
G_HA_TMP_PATH=/usr/local/ha/local/tmp
G_SUDO_SCRIPT_PATH=/opt/script
source ${G_HA_SCRIPT_PATH}/log.sh

G_EVENT_CODE_FILE=${G_HA_SCRIPT_PATH}/conf/event_code.xml
G_ALARM_OPERATION_FILE=${G_HA_TMP_PATH}/alarm.op

#Event ID
G_EVENT_ID_HEARTBEAT_LOST="EVENT_HA_HEARTBEAT_LOST"
G_EVENT_ID_FILE_SYNC_FAILED="EVENT_HA_FILE_SYNC_FAILED"
G_EVENT_ID_SWITCHOVER="EVENT_HA_SWITCH"
G_EVENT_ID_GATEWAY_NOT_REACH="EVENT_HA_GATEWAY_CANNOT_REACH"


##资源类告警
G_EVENT_ID_STATUS_EXCEPTION="EVENT_HA_STATUS_EXCEPTION"
G_EVENT_ID_START_FAIL="EVENT_HA_START_FAIL"
G_EVENT_ID_STOP_FAIL="EVENT_HA_STOP_FAIL"
G_EVENT_ID_FORCE_STOP_FAIL="EVENT_HA_FORCE_STOP_FAIL"
G_EVENT_ID_DEACTIVE_FAIL="EVENT_HA_DEACTIVE_FAIL"
G_EVENT_ID_ACTIVE_FAIL="EVENT_HA_ACTIVE_FAIL"
G_EVENT_ID_FLOAT_IP_CONFLICT="EVENT_HA_FLOAT_IP_CONFLICT"

G_UNDEFINED_STRING="undefined"

function get_ha_mode()
{
    local ha_mode=$(sed -n 's/.*hamode.*value=\"\(.*\)\".*/\1/p' ${G_HA_INSTALL_PATH}/module/hacom/conf/hacom.xml)
    [ "X" == "X${ha_mode}" ] && ha_mode="${G_UNDEFINED_STRING}"
    echo "$ha_mode"
}

function get_gateway_ip()
{
    local gw_ip=""
    local start_line=$(cat ${G_HA_INSTALL_PATH}/module/haarb/conf/haarb.xml | grep -n "<gateway>" | cut -d ":" -f1)
    local end_line=$(cat ${G_HA_INSTALL_PATH}/module/haarb/conf/haarb.xml | grep -n "</gateway>" | cut -d ":" -f1)
    if [ $start_line == $end_line ]; then
        local ip_number=$(cat ${G_HA_INSTALL_PATH}/module/haarb/conf/haarb.xml | grep gateway | grep -o "[0-9]\{1,3\}[.][0-9]\{1,3\}[.][0-9]\{1,3\}[.][0-9]\{1,3\}" | wc -l)
        for (( i = 1; i <= ip_number; i++ )); do
            local ip=$(cat ${G_HA_INSTALL_PATH}/module/haarb/conf/haarb.xml | grep gateway | grep -o "[0-9]\{1,3\}[.][0-9]\{1,3\}[.][0-9]\{1,3\}[.][0-9]\{1,3\}" | sed -n ${i}p)
            if [ -z ${gw_ip} ]; then
                gw_ip=$ip
            else
                gw_ip="$gw_ip|$ip"
            fi
        done
    else
        for (( i = start_line; i <= end_line; i++ )); do
            local ip=$(sed -n ${i}p ${G_HA_INSTALL_PATH}/module/haarb/conf/haarb.xml | grep -o "[0-9]\{1,3\}[.][0-9]\{1,3\}[.][0-9]\{1,3\}[.][0-9]\{1,3\}")
            if [ -z ${ip} ]; then
                continue
            fi
            if [ -z ${gw_ip} ]; then
                gw_ip=$ip
            else
                gw_ip="$gw_ip|$ip"
            fi
        done
    fi
    [ "X" == "X${gw_ip}" ] && gw_ip="${G_UNDEFINED_STRING}"
    echo "${gw_ip}"
}

function get_float_ip()
{
    local float_ip=$(sed -n "s/.*arpip.*value=\"\(.*\)\".*/\1/p" ${G_HA_INSTALL_PATH}/module/haarb/conf/haarb.xml)
    [ "X" == "X${float_ip}" ] && float_ip="${G_UNDEFINED_STRING}"
    echo "${float_ip}"
}

function get_pair_ip()
{
    local pair_ip=$(sed -n "s/.*node.*ip=\"\(.*\)\" port.*/\1/p" ${G_HA_INSTALL_PATH}/module/haarb/conf/haarb.xml)
    [ "X" == "X${pair_ip}" ] && pair_ip="${G_UNDEFINED_STRING}"
    echo "${pair_ip}"
}

function get_local_name()
{
    local local_name=$(sed -n 's/.*local name=\"\(.*\)\".*/\1/p' ${G_HA_INSTALL_PATH}/local/hacom/conf/hacom_local.xml)
    [ "X" == "X${local_name}" ] && local_name="${G_UNDEFINED_STRING}"
    echo "${local_name}"
}

function get_peer_name()
{
    local peer_name=$(sed -n 's/.*peer name=\"\(.*\)\".*/\1/p' ${G_HA_INSTALL_PATH}/local/hacom/conf/hacom_local.xml)
    [ "X" == "X${peer_name}" ] && peer_name="${G_UNDEFINED_STRING}"
    echo "${peer_name}"
}

##
function get_ip_by_ha_name()
{
    local ha_name=$1
    local ip=$(sed -n "s/.*${ha_name}.*ip=\"\(.*\)\".*port=.*/\1/p" ${G_HA_INSTALL_PATH}/module/haarb/conf/haarb.xml)
    [ "X" == "X${ip}" ] && ip="${G_UNDEFINED_STRING}"
    echo "${ip}"
}

function add_ha_ip_rule() {
    local_name=$(get_local_name)
    local_ip=$(get_ip_by_ha_name ${local_name})
    table_id=`ip rule | grep "from $local_ip lookup" | awk -F ' ' '{print $5}'`
    local peer_name=$(get_peer_name)
    local peer_ip=$(get_ip_by_ha_name ${peer_name})
    local gw_ips=$(get_gateway_ip)
    local float_ip=$(get_float_ip)
    sudo ip rule add from all to $peer_ip lookup $table_id
    sudo ip rule add from all to $float_ip lookup $table_id
    sudo ip rule add from all to $gw_ips lookup $table_id
}

function get_role()
{
    #获取主备状态
    local role=$(sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh ha_role)
    [ "X" == "X${role}" ] && role="${G_UNDEFINED_STRING}"
    echo "${role}"
}

function get_event_id()
{
    eval `grep "$1" $G_EVENT_CODE_FILE | sed -n 's/.*\(event_id=\".*\"\)[ ]*severity.*/\1/p'`

}

function get_severity()
{
    eval `grep "$1" $G_EVENT_CODE_FILE | sed -n 's/.*\(severity=\".*\"\)[ ]*macro.*/\1/p'`
}

function get_parameters()
{
   ### $1 event name .e.g: EVENT_HA_STATUS_EXCEPTION
   ### $2 resource
   ##EventID number .e.g: 0x000200220004
   get_event_id $1
   [ "X" == "X${event_id}" ] && event_id="${G_UNDEFINED_STRING}"
   ##Serverity
   get_severity $1
   [ "X" == "X${severity}" ] && severity="${G_UNDEFINED_STRING}"

   local_resource_name=$2
   local_name=$(get_local_name)
   local peer_name=$(get_peer_name)
   local_ip=$(get_ip_by_ha_name ${local_name})
   local peer_ip=$(get_ip_by_ha_name ${peer_name})
   local local_role=$(get_role ${local_name})
   local gw_ips=$(get_gateway_ip)
   local float_ip=$(get_float_ip)

   parameters=" "
   case $1 in
        ${G_EVENT_ID_HEARTBEAT_LOST})
            if [ "${local_role}" == "standby" ]; then
                parameters=${peer_name},${peer_ip},${local_name},${local_ip}
            else
                parameters=${local_name},${local_ip},${peer_name},${peer_ip}
            fi
            ;;
        ${G_EVENT_ID_FILE_SYNC_FAILED})
            if [ "${local_role}" == "standby" ]; then
                parameters=${peer_name},${peer_ip},${local_name},${local_ip}
            else
                parameters=${local_name},${local_ip},${peer_name},${peer_ip}
            fi
            ;;
        ${G_EVENT_ID_SWITCHOVER})
            if [ "${local_role}" == "standby" ]; then
                parameters=${peer_name},${local_name}
            else
                parameters=${local_name},${peer_name}
            fi
            ;;
        ${G_EVENT_ID_GATEWAY_NOT_REACH})
            parameters=${local_name},${gw_ips}
            ;;
        ${G_EVENT_ID_STATUS_EXCEPTION})
            parameters=${local_name},${local_ip},${local_resource_name}
            ;;
        ${G_EVENT_ID_DEACTIVE_FAIL})
            parameters=${local_name},${local_ip},${local_resource_name}
            ;;
        ${G_EVENT_ID_ACTIVE_FAIL})
            parameters=${local_name},${local_ip},${local_resource_name}
            ;;
        ${G_EVENT_ID_FLOAT_IP_CONFLICT})
            parameters=${float_ip}
            ;;
        *)
            ;;
    esac

}

function process_ha_alarm()
{
    ##$1: type, e.g.: create, recover, event
    ##$2: event name, e.g.: EVENT_HA_HEARTBEAT_LOST
    ##$3: resource, e.g.: gaussdb, floatIp...
    get_parameters  "$2" "$3"
    log_info "[${FUNCNAME[0]},$LINENO]start send HA alarm, alarm type:$1,event ID :${event_id},node name:${local_name},local IP:${local_ip},parameter list:${parameters},severity :${severity}"
    local current=`date "+%Y-%m-%d %H:%M:%S"`
    local timestamp=`date -d "$current" +%s`
    local current_timestamp=$(((timestamp*1000+10#`date "+%N"`/1000000)/1000)) #将current转换为时间戳，精确到秒
    log_info "[${FUNCNAME[0]},$LINENO]start send HA alarm, current=$current, timestamp=$timestamp, current_timestamp=$current_timestamp."
    ##create 0x000200220004 gaussdb:abc:def:aa:bbb 3 1684915025
    ##告警写入alarm.op文件
    if [ ! -f "${G_ALARM_OPERATION_FILE}" ]; then
        touch "${G_ALARM_OPERATION_FILE}"
        chmod 660 ${G_ALARM_OPERATION_FILE}
    fi
    echo "$1 ${event_id} ${parameters} ${severity} ${current_timestamp}" >> ${G_ALARM_OPERATION_FILE}
    if [ $? != 0 ]; then
        log_error "[${FUNCNAME[0]},$LINENO]write alarm info error"
    fi
    cd -
}