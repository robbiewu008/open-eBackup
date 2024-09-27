#!/bin/bash

if [ "$__INSTALL_FUNCTION_SH__" == "" ]; then

    # 输出的日志级别
    declare -i ERROR="2"
    declare -i WARN="1"
    declare -i INFO="0"
    declare -i DEBUG_LEVEL="0"
    declare -a LOG_LEVEL=("INFO" "WARN" "ERROR")


    # 从数据库中产出的状态值
    declare -r gs_s_normal="Normal"                         # 数据库双机状态正常
    declare -r gs_s_unknown="Unknown"                       # 未知的双机状态
    declare -r gs_s_needrepair="Needrepair"                 # 需要重建状态
    declare -r gs_s_starting="Starting"                     # 数据库正在启动
    declare -r gs_s_waiting="Waiting"                       # 等待主机降备（备才有的状态）
    declare -r gs_s_demoting="Demoting"                     # 主机正在进行降备（主才有这个而状态）
    declare -r gs_s_promoting="Promoting"                   # 被正在升主（备才有的状态）
    declare -r gs_s_rebuilding="Rebuilding"                 # 被正在进行重建（备才有的状态）

    # 从数据库中查出的数据库角色值
    declare -r gs_r_primary="Primary"                       # 本端数据库为主
    declare -r gs_r_standby="Standby"                       # 本端数据库为备
    declare -r gs_r_cstandby="CascadeStandby"               # 本端数据库为级联备
    declare -r gs_r_pending="Pending"                       # 数据库处于等待，此时可等待主机命令，成为主或备。
    declare -r gs_r_normal="Normal"                         # 本端数据库为单机
    declare -r gs_r_unknown="Unknown"                       # 本端数据库以未知方式使用

    # 从数据库中查询详细重建原因
    declare -r gs_d_normal="Normal"                         # 双机关系正常，不需要重建
    declare -r gs_d_connecting="Connecting"                 # 正在尝试进行重建
    declare -r gs_d_disconnected="Disconnected"             # 未连接
    declare -r gs_d_walremoved="WALsegmentremoved"          # 日志段已经删除
    declare -r gs_d_vernotmatched="Versionnotmatched"       # 版本不匹配
    declare -r gs_d_modenotmatched="Modenotmatched"         # 模式不匹配
    declare -r gs_d_sysIDnotmatched="Systemidnotmatched"    # 数据不通源，双机间的数据目录不是同一个数据库初始化创建的
    declare -r gs_d_timenotmatched="Timelinenotmatched"     # 时间线不匹配

    # 启动数据库时，指定的角色
    declare -r gs_c_pending="pending"                       #
    declare -r gs_c_primary="primary"                       #
    declare -r gs_c_standby="standby"                       #
    declare -r gs_c_normal="normal"                         #

    declare -i r_success=0
    declare -i r_failure=1

    # 高斯能够进行重建的原因
    declare -r gs_r_canrepair="$gs_d_walremoved|$gs_d_sysIDnotmatched"

    declare -r gs_notneedinfo="SENDER_SENT_LOCATION|SENDER_WRITE_LOCATION|SENDER_FLUSH_LOCATION|SENDER_REPLAY_LOCATION|RECEIVER_RECEIVED_LOCATION|RECEIVER_WRITE_LOCATION|RECEIVER_FLUSH_LOCATION|RECEIVER_REPLAY_LOCATION"

    # 返回码
    declare -i db_normal=0           #   正常运行
    declare -i db_abnormal=1         #   运行异常
    declare -i db_stopped=2          #   停止
    declare -i db_unknown=3          #   状态位置
    declare -i db_starting=4         #   正在启动
    declare -i db_stopping=5         #   正在停止
    declare -i db_primary=6          #   主正常运行
    declare -i db_standby=7          #   备正常运行
    declare -i db_activating=8       #   正在升主
    declare -i db_deactivating=9     #   正在降备
    declare -i db_notsupported=10    #   动作不存在
    declare -i db_repairing=11       #   正在重建

    # 返回值对应的字符串
    declare -a out_ret_string=("normal" "abnormal" "stopped" "unknown" "starting" "stopping"\
                             "primary" "standby" "activating" "deactivating" "notsupported" "repairing")

    declare gs_cur_mode=""                    # 当前数据库配置
    declare gs_db_single="single"             # 高斯DB，单机配置
    declare gs_db_double="double"             # 高斯DB，双机配置

    # 脚本位置
    G_HA_TMP_PATH=/usr/local/ha/local/tmp
    G_HA_SCRIPT_PATH=/usr/local/ha/script
    source $G_HA_SCRIPT_PATH/log.sh
    G_SUDO_SCRIPT_PATH=/opt/script
    LOG_FILE="${LOG_PATH}/ha.log"
    # 数据库相关
    DBInstallPath=/usr/local/gaussdb/app
    PGDATA=${DBInstallPath}/data
    PGCTL="${DBInstallPath}/app/bin/gs_ctl -D ${PGDATA}"
    GAUSSDB_CONNECT_PORT=30170
    export LD_LIBRARY_PATH=${DBInstallPath}/app/lib
    export GAUSSHOME=${DBInstallPath}
    export GAUSSDATA=${PGDATA}
    # 下面是做特殊处理的临时文件
    declare START_FAIL_RECORD="${G_HA_TMP_PATH}/.startGS.fail"                      # 数据库启动失败记录
    declare -i TIMES_TO_REBUILD=3              # 连续启动失败后，重建

    __INSTALL_FUNCTION_SH__='initialized'
fi


######################################################################
#   FUNCTION   : log
#   DESCRIPTION: 打印错误
######################################################################
log()
{
echo "$@"
    local  log_level=$1
    local  func_name=$2
    local  line_num=$3
    local  Log_content=$4

    case $log_level in
        "0")
            log_info "[$func_name(),$line_num]$Log_content"
        ;;
        "1")
            log_warn "[$func_name(),$line_num]$Log_content"
        ;;
        "2")
            log_error "[$func_name(),$line_num]$Log_content"
        ;;
    esac
}

######################################################################
#   FUNCTION   : omm_exit
#   DESCRIPTION: 退出函数，用于退出脚本，并输出日志
######################################################################
omm_exit()
{
    local -i ret_val=$1
    log "$WARN" "${FUNCNAME[0]}" "$LINENO" "Exit: $ret_val[${out_ret_string[$ret_val]}]."
    exit $ret_val
}

######################################################################
#   FUNCTION   : is_primary
#   DESCRIPTION: 检查当前调用的脚本的HA角色是否为主
######################################################################
is_primary()
{
    if [ "$RUN_STATE" == "active" ]; then
        return $r_success
    fi

    return $r_failure
}

######################################################################
#   FUNCTION   : is_db_running
#   DESCRIPTION: 判断数据库是否在运行
#   0 为正在运行
######################################################################
is_db_running()
{
    local -i ret_val=0
    local dbinfo=""
    local dbresult=""

    # 获取数据库运行状态，判断是否正在运行
    dbinfo=$(sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_status)

    # 数据库正在运行
    dbresult=$(echo "$dbinfo" | grep -w "server is running" ); ret_val=$?
    if [ $ret_val -eq $r_success ] ; then
        log "$INFO" "${FUNCNAME[0]}" "$LINENO" "db is running now. [$dbresult]."
        return $r_success
    fi

    # 数据库不在运行中
    log "$INFO" "${FUNCNAME[0]}" "$LINENO" "db is not running now. [$dbinfo]."
    return $r_failure
}


######################################################################
#   FUNCTION   : get_db_status_info
#   DESCRIPTION: 获取数据库状态信息，去除不需要的信息
#   0 为正在运行
######################################################################
get_db_status_info()
{
    sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_query
}

######################################################################
#   FUNCTION   : get_db_state
#   DESCRIPTION: 获取数据库状态，从数据库的查询结果中分理处各项
#                local_role, db_state, detail_information, peer_role
######################################################################
get_db_state()
{
    local dbinfo="$1"

    local_role=""; db_state=""; detail_information=""; peer_role="";

    # 直接从结果中转换，未做任何处理
    eval $(echo "$dbinfo" | grep -Ew "local_role"         | sed -e 's/:/=/g' | sed -e 's/ //g' | sed -n '1p')
    eval $(echo "$dbinfo" | grep -Ew "db_state"           | sed -e 's/:/=/g' | sed -e 's/ //g' | sed -n '1p')
    eval $(echo "$dbinfo" | grep -Ew "detail_information" | sed -e 's/:/=/g' | sed -e 's/ //g' | sed -n '1p')
    eval $(echo "$dbinfo" | grep -Ew "peer_role"          | sed -e 's/:/=/g' | sed -e 's/ //g' | sed -n '1p')

    return $r_success
}

######################################################################
#   FUNCTION   : check_local_role
#   DESCRIPTION: 检查数据库本地角色是否为指定角色
######################################################################
check_local_role()
{
    local check_role="$1"
    local dbinfo=""

    # 获取数据库状态
    dbinfo=$(get_db_status_info)
    if ! echo "$dbinfo" | grep -w "local_role" | grep -wE "$check_role"; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[check_local_role] get role[$check_role] failure. [$dbinfo]"
        return $r_failure
    fi

    return $r_success
}


######################################################################
#   FUNCTION   : is_need_rebuild
#   DESCRIPTION: 判断能否进行重建
#####################################################################
is_need_rebuild()
{
    local -i fail_count=0
    local build_str=""
    build_str=$(sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_querybuild 2>&1)

    log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[is_need_rebuild] querybuild result. [$build_str]"

    # 如果重建中则不需要处理
    if echo "$build_str" | grep -w "Building" > /dev/null; then
        log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[is_need_rebuild] DB is rebuilding now."
        return $db_repairing
    fi


    # 当前没有重建，则判断启动次数是否超过最大次数
    fail_count=$(cat $START_FAIL_RECORD)
    if [ $fail_count -gt $TIMES_TO_REBUILD ]; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[is_need_rebuild] DB start failure for $fail_count times, need reparid."
        return $db_abnormal
    fi

    return $db_normal
}


######################################################################
#   FUNCTION   : record_start_result
#   DESCRIPTION: 记录启动结果，主要是对失败结果进行记录
#                对于备数据库，一直没有启动成功，需要重建
#####################################################################
record_start_result()
{
    local -i sub_ret=$1
    local -i fail_count=0

    # 启动失败，记录失败次数
    if [ $sub_ret -ne $r_success ];then
        fail_count=$(cat $START_FAIL_RECORD)
        fail_count=$fail_count+1
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[record_start_result] DB start failure for $fail_count times."
    fi

    echo $fail_count > $START_FAIL_RECORD

    return $r_success
}

######################################################################
#   FUNCTION   : restart_to_normal
#   DESCRIPTION: 将数据库重启为正常角色操作
######################################################################
restart_to_normal()
{
    local -i ret_val=$r_success

    restart_to_state "$gs_r_normal" "$gs_c_normal"; ret_val=$?
    record_start_result $ret_val

    return $ret_val
}

######################################################################
#   FUNCTION   : restart_to_pending
#   DESCRIPTION: 将数据库重启为pending角色
######################################################################
restart_to_pending()
{
    local -i ret_val=0

    restart_to_state "$gs_r_pending" "$gs_c_pending" ; ret_val=$?
    record_start_result $ret_val

    return $ret_val
}

######################################################################
#   FUNCTION   : restart_to_primary
#   DESCRIPTION: 将数据库重启为主
######################################################################
restart_to_primary()
{
    local -i ret_val=0

    restart_to_state "$gs_r_primary" "$gs_c_primary" ; ret_val=$?
    record_start_result $ret_val

    return $ret_val
}

######################################################################
#   FUNCTION   : restart_to_standby
#   DESCRIPTION: 将数据库重启为备
######################################################################
restart_to_standby()
{
    local -i ret_val=0

    restart_to_state "$gs_r_standby" "$gs_c_standby" ; ret_val=$?
    record_start_result $ret_val

    return $ret_val
}

######################################################################
#   FUNCTION   : restart_to_state
#   DESCRIPTION: 重启数据库到目标状态
#   需要输入重启后的检查状态，和重启后的状态
######################################################################
restart_to_state()
{
    if [ -L ${LOG_FILE} ];then
        echo "${LOG_FILE} is symbolic link file, program exit"
        return 1
    fi

    local chk_state="$1"           # 启动后检查状态
    local sta_state="$2"           # 启动时的状态
    local -i ret_val=0

    log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[restart_to_state] start to restart db for $sta_state, and check state is $chk_state."

    # 停止数据库，停止失败则强制停止
    sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_stop fast >>"$LOG_FILE" 2>&1 || sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_stop immediate >>"$LOG_FILE" 2>&1

    # 启动数据库为目标状态
    if [ "$sta_state" == "$gs_c_normal" ]; then
	      ps -ef | grep "gs_ctl.*start" | grep -v "grep" >> $LOG_FILE 2>&1
	      if [ $? -ne 0 ];then
            sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_start >>"$LOG_FILE" 2>&1; ret_val=$?
        else
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[restart_to_state] A start task is running."
        fi
    else
	      ps -ef | grep "gs_ctl.*start" | grep -v "grep" >> $LOG_FILE 2>&1
	      if [ $? -ne 0 ];then
	          sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_start $sta_state >>"$LOG_FILE" 2>&1; ret_val=$?
        else
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[restart_to_state] A start task is running."
        fi
    fi

    # 数据库未启动
    if [ $ret_val -ne $r_success ]; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[restart_to_state] call ($PGCTL start [-M $sta_state]) failure[$ret_val]."
        return $r_failure
    fi

    # 启动后，需要检查当前数据库是否已经启动
    if ! check_local_role "$chk_state" ; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[restart_to_state] call ($PGCTL start [-M $sta_state]) success, but db still not [$chk_state]."
        return $r_failure
    fi

    log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[restart_to_state] success to restart db for $sta_state, and check state is $chk_state."
    return $r_success
}

######################################################################
#   FUNCTION   : notify_to_primary
#   DESCRIPTION: 通知数据库角色转换为主的操作
######################################################################
notify_to_primary()
{
    local -i ret_val=0

    # 通知数据库，将状态切换为主
    sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_notify $gs_c_primary;ret_val=$?

    # 通知失败
    if [ $ret_val -ne $r_success ]; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[notify_to_primary] call [$PGCTL notify -M $gs_c_primary] failure[$ret_val]."
        return $r_failure
    fi

    return $r_success
}

######################################################################
#   FUNCTION   : notify_to_standby
#   DESCRIPTION: 通知数据库角色转换为备的操作
######################################################################
notify_to_standby()
{
    local -i ret_val=0
    # 通知数据库，将状态切换为备
    sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_notify $gs_c_standby;ret_val=$?

    # 通知失败
    if [ $ret_val -ne $r_success ]; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[notify_to_standby] call [$PGCTL notify -M $gs_c_standby] failure[$ret_val]."
        return $r_failure
    fi

    sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_build ; ret_val=$?
    # 重建失败
    if [ $ret_val -ne $r_success ]; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[notify_to_build] call [$PGCTL build failure[$ret_val]."
        return $r_failure
    fi
    return $r_success
}

######################################################################
#   FUNCTION   : sta_to_pri
#   DESCRIPTION: 备升主的操作，switchover，失败后尝试强制升主
######################################################################
sta_to_pri()
{
    local -i ret_val=0

    sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_switchover fast;ret_val=$?

    # 切换失败，命令执行失败或是状态没有切换为主用
    if [ $ret_val -ne $r_success ] || ! check_local_role "$gs_r_primary"; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[notify_to_standby] call [./db_tool -o switchover] failure[$ret_val], try failover."

        # 是否需要尝试多几次
        sta_force_to_pri; ret_val=$?
        return $ret_val
    fi

    # 切换成功
    return $r_success
}


######################################################################
#   FUNCTION   : sta_force_to_pri
#   DESCRIPTION: 被强制升主，failover
######################################################################
sta_force_to_pri()
{
    local -i ret_val=0

    sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_switchover immediate;ret_val=$?

    # 切换失败，命令执行失败或状态没有切换为主
    if [ $ret_val -ne $r_success ] || ! check_local_role "$gs_r_primary"; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[notify_to_standby] call switchover failure[$ret_val], It can't be helped."
        return $r_failure
    fi

    return $r_success
}

######################################################################
#   FUNCTION   : reload_float_ip
#   DESCRIPTION: 备升主，高斯无法加载配置 OMM脚本规避
######################################################################
reload_float_ip()
{
    return 0
}

######################################################################
#   FUNCTION   : pri_to_sta
#   DESCRIPTION: 主降备，升级就是重启为备
######################################################################
pri_to_sta()
{
    local -i ret_val=0

    restart_to_standby; ret_val=$?

    return $ret_val
}

######################################################################
#   FUNCTION   : config_db_link
#   DESCRIPTION: 配置数据库链路信息
#   需要输入本端IP，对端IP
######################################################################
config_db_link()
{
    internal_ip=$1
    connect_ip=$2
    sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_config link ${internal_ip} ${GAUSSDB_CONNECT_PORT} ${connect_ip} ${GAUSSDB_CONNECT_PORT}
    return $?
}

######################################################################
#   FUNCTION   : clear_db_link
#   DESCRIPTION: 清楚数据库链路配置信息
#   不需要输入
######################################################################
clear_db_link()
{
    sudo /opt/script/ha_sudo.sh db_config clear
    return $?
}