#!/bin/bash

# 整个数据库监控的逻辑如下：
#   --------------------------------- 单机 ---------------------------------  #
#   如果数据库不再位，则返回停止，由启动脚本启动。
#   如果数据库角色不正确，且不是中间态，则返回异常，由修复脚本修复。
#   修复时，如果在中间则本次不处理，等待重建完成后再修复。
#   ========================================================================  #
#   --------------------------------- 双机---------------------------------  #
#
#   ========================================================================  #

G_HA_SCRIPT_PATH=/usr/local/ha/script
G_SUDO_SCRIPT_PATH=/opt/script
G_SERVICE_NAME=$G_RESOURCE_NAME

if ! . "${G_HA_SCRIPT_PATH}/dbfunc.sh" ; then
    echo "[$(date)]ERROR: load ${G_HA_SCRIPT_PATH}/dbfunc.sh failure!"
    exit 1
fi

if ! . "${G_HA_SCRIPT_PATH}/event_lib.sh" ; then
    echo "[$(date)]ERROR: load ${G_HA_SCRIPT_PATH}/event_lib.sh failure!"
    exit 1
fi


######################################################################
#   FUNCTION   : get_status
#   DESCRIPTION: 获取数据库状态
######################################################################
get_status()
{
    # 数据库的状态检查大体步骤，双击和单机模式有一些差异
    # Step1: 数据库不在位，需要返回不在位
    # Step2: 数据库在位，检查是否pending，是则返回与期望状态相反
    # Step3: 数据库在位，处于中间状态，返回相应的中间态
    # Step4: 数据库在位，检查是否需要重建，如果需要重建，则数据库角色为备返回需要重建
    # Step5: 数据库在位，不需要重建，如果数据库角色正确，则返回ok
    # Step6: 数据库在位，数据库角色不正确，返回需要激活或去激活
    local -i ret_val=$r_success

    if [ "$gs_cur_mode" == "$gs_db_single" ]; then
        get_status_single ; ret_val=$?
    else
        get_status_double ; ret_val=$?
    fi

    return $ret_val
}

######################################################################
#   FUNCTION   : get_status_single
#   DESCRIPTION: 单机数据库状态查询函数
#   脚本返回码:
#   单主模式或双主模式
#    declare -i db_normal=0           #   正常运行
#    declare -i db_abnormal=1         #   运行异常
#    declare -i db_stopped=2          #   停止
#    declare -i db_unknown=3          #   状态未知
#    declare -i db_starting=4         #   正在启动
#    declare -i db_stopping=5         #   正在停止
#    declare -i db_primary=6          #   主正常运行
#    declare -i db_standby=7          #   备正常运行
#    declare -i db_activating=8       #   正在升主
#    declare -i db_deactivating=9     #   正在降备
#    declare -i db_notsupported=10    #   动作不存在
#    declare -i db_repairing=11       #   正在修复
######################################################################
get_status_single()
{
    local dbinfo=""
    local -i ret_val=$r_success

    # 查询数据库是否正常运行
    # 数据库未启动
    is_db_running
    if [[ $? != 0 ]]; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db no runnig now."
        return $db_stopped
    fi

    # 获取数据库状态
    dbinfo=$(get_db_status_info)

    # 从数据库的查询结果中分离出各项信息: local_role, db_state, detail_information, peer_role. 前面四个变量会在get_db_state赋值。
    get_db_state "$dbinfo"; ret_val=$?             # 执行命令，立即获取返回值。写在同一行，是为了避免后续修改时，在两个语句中插入其他命令，导致获取失败。

    # 根据数据库角色进行判断，正常情况下，只会出现Normal和空（即获取失败）
    case "$local_role" in
        "$gs_r_normal")
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db current role Normal. [$dbinfo]"
            return $db_normal
            ;;

        # 以下三种状态，虽然在单机下不可能出现，但是需要进行异常处理，这三种状态下，先判断是否在重建中。
        "$gs_r_primary"|"$gs_r_standby"|"$gs_r_cstandby")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db current role error[$local_role], need further check. [$dbinfo]"
            ;;

        # Pending角色下的数据库，可以直接状态变更，所以返回未启动。
        "$gs_r_pending")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db current role Pending, need restart. [$dbinfo]"
            return $db_abnormal
            ;;

        # 其他角色不是数据库的正常角色，可能是获取失败了，认为数据库未启动。
        "$gs_r_unknown"|*)
            log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db current role[$local_role] error! [$dbinfo]"
            return $db_abnormal
            ;;
    esac

    # 以下判断是在本段数据库角色为主、备、级联备的情况的判断，属于异常处理
    case "$db_state" in
        # 以下几种状态，表示数据库状态不稳定，待稳定后再进行下一步处理，避免出现问题。
        "$gs_s_rebuilding")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $db_repairing
            ;;

        "$gs_s_demoting")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $db_deactivating
            ;;

        "$gs_s_waiting"|"$gs_s_promoting")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $db_activating
            ;;

        "$gs_s_starting")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $db_starting
            ;;

        # 其他状态，直接重启为单机
        "$gs_s_normal"|"$gs_s_needrepair"|"$gs_s_unknown"|*)
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_single] db state[$db_state] stable now, need restart. [$dbinfo]"
            return $db_abnormal
            ;;
    esac

    # 数据库正常运行
    return $db_normal
}

######################################################################
#   FUNCTION   : get_status_single
#   DESCRIPTION: 单机数据库状态查询函数
#   脚本返回码:
#   单主模式或双主模式
#    declare -i db_normal=0           #   正常运行
#    declare -i db_abnormal=1         #   运行异常
#    declare -i db_stopped=2          #   停止
#    declare -i db_unknown=3          #   状态未知
#    declare -i db_starting=4         #   正在启动
#    declare -i db_stopping=5         #   正在停止
#    declare -i db_primary=6          #   主正常运行
#    declare -i db_standby=7          #   备正常运行
#    declare -i db_activating=8       #   正在升主
#    declare -i db_deactivating=9     #   正在降备
#    declare -i db_notsupported=10    #   动作不存在
#    declare -i db_repairing=11       #   正在修复
######################################################################
get_status_double()
{
    local dbinfo=""
    local -i ret_val=0
    local -i out_result=$r_success                    # 返回的结果，用于在主备情况下暂存结果。

    # 查询数据库是否正常运行
    # 数据库未启动
    is_db_running
    if [[ $? != 0 ]]; then
        log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db no runnig now."

        # 如果HA期待的数据库角色为主，插件认DB正确就应为主，此时启动为主
        if [ "$RUN_STATE" == "active" ]; then
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] HA wantted primary,DB no runnig,restart to primary!!!"
            restart_to_primary;ret_val=$?
            return $db_stopped
        fi

        # 数据库没有运行，可能是无法启动，检查是否需要重建
        is_need_rebuild; ret_val=$?
        [ $ret_val -eq $db_normal ] || return $ret_val
        return $db_stopped
    fi

    # 获取数据库状态
    dbinfo=$(get_db_status_info)

    # 从数据库的查询结果中分离出各项信息: local_role, db_state, detail_information, peer_role. 前面四个变量会在get_db_state赋值。
    get_db_state "$dbinfo"; ret_val=$?

    # 根据数据库角色进行判断，正常时只会出现Pending，Primary，Standby和空（获取失败）
    case "$local_role" in
        # Pending状态下的数据库，可以直接状态变更，所以返回期望该状态的反状态
        "$gs_r_pending")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db current role[$local_role], current ha role[$RUN_STATE]."
            return $db_normal
            ;;

        # 以下两种状态，需要先判断是否正在重建
        "$gs_r_primary")
            out_result=$db_primary
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db current role[$local_role], need further check."
            ;;

        "$gs_r_standby")
            out_result=$db_standby
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db current role[$local_role], need further check."
            ;;

        # 以下角色不是双机会出现的角色，直接重启即可。
        "$gs_r_normal"|"$gs_r_cstandby"|"$gs_r_unknown"|*)
            log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db current role[$local_role] error! [$dbinfo]"
            return $db_stopped
            ;;
    esac

    # 当前为HA的状态未知时，数据库主备需要返回为正常
    [ "$RUN_STATE" == "unknown" ] && out_result=$db_normal

    # 以下判断是在本端数据库角色为主、备、级联备的情况，需要判断数据库状态。
    case "$db_state" in
        # 以下几种状态，表示数据库状态不稳定，待稳定后再进行下一步处理，避免出现问题。
        "$gs_s_rebuilding")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $db_abnormal #HA advice when standby is repairing, return excepion to it.
            ;;

        "$gs_s_demoting")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $db_deactivating
            ;;

        "$gs_s_waiting"|"$gs_s_promoting")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $db_activating
            ;;

        "$gs_s_starting")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $db_starting
            ;;

        # 数据库需要重建，则需要考虑对端情况
        "$gs_s_needrepair")
            # 重建状态不一定是需要修复的
            if echo "$detail_information" | grep -Ew "$gs_r_canrepair"; then
                log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db state[$db_state] need repair now, and current role[$local_role]. [$dbinfo]"
                # 如果数据库角色为主，则先不处理
                if [ "$local_role" == "Primary" ];then
                    return $db_activating
                else
                # 否则返回需要修复
                    return $db_abnormal
                fi
            else
                log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db state[$db_state] need repair now, but no need to repair, detail_information[$detail_information]. [$dbinfo]"
                return $out_result
            fi
            ;;

        # 其他状态，直接重启跳转到需要的状态
        "$gs_s_normal"|"$gs_s_unknown"|*)
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[get_status_double] db state[$db_state] stable now, can goto want role."
	          local out_info=""
            return $out_result
            ;;
    esac

    # 数据库正常运行，返回当前状态
    return $db_normal
}


######################################################################
#   FUNCTION   : do_start
#   DESCRIPTION: 数据库启动函数
######################################################################
do_start()
{
    local -i ret_val=$r_success

    # 未启动，则根据单机双机配置启动为相应的模式
    if [ "$gs_cur_mode" == "$gs_db_single" ]; then
        do_start_single ; ret_val=$?
    else
        do_start_double ; ret_val=$?
    fi

    return $ret_val
}

######################################################################
#   FUNCTION   : do_start_single
#   DESCRIPTION: 单机数据库启动函数，只将数据启动为Normal
######################################################################
do_start_single()
{
    local -i ret_val=$r_success

    # 启动前，先检查当前数据库是否已经启动为Normal
    if check_local_role "Normal"; then
        log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_start_single] db had been start already."
        return $r_success
    fi

    # 启动数据库
    restart_to_normal; ret_val=$?

    return $ret_val
}

######################################################################
#   FUNCTION   : do_start_double
#   DESCRIPTION: 双机数据库启动函数
######################################################################
do_start_double()
{
    local -i ret_val=$r_success

    # 启动前，先检查当前数据库是否已经启动为Primary|Standby|Pending
    if check_local_role "Primary|Standby|Pending"; then
        log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_start_double] db had been start already."
        return $r_success
    fi

    # 启动数据库
    restart_to_pending; ret_val=$?

    return $ret_val
}

######################################################################
#   FUNCTION   : do_stop
#   DESCRIPTION: 数据库停止函数
######################################################################
do_stop()
{
    local -i ret_val=$r_success

    # 停止前，先检查数据库是否已经停止
    is_db_running
    if [[ $? != 0 ]]; then
        log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[doStop]db is not running now."
        return $r_success
    fi

    # 停止数据库
    dbinfo=$(sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_stop fast); ret_val=$?

    return $r_success
}

######################################################################
#   FUNCTION   : do_stop_force
#   DESCRIPTION: 强制停止数据库
######################################################################
do_stop_force()
{
    local -i ret_val=$r_success

    # 停止前，先检查当前数据库是否已经停止
    is_db_running
    if [[ $? != 0 ]]; then
        log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_stop_force]db is not running now."
        return $r_success
    fi

    # 强制停止数据库
    dbinfo=$(sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_stop immediate); ret_val=$?

    return $r_success
}

######################################################################
#   FUNCTION   : do_activate
#   DESCRIPTION: 数据库激活函数
######################################################################
do_activate()
{
    local out_info=""
    ##process_database_sync_abnormal 1
    # 本端没有启动，尝试启动为主
    is_db_running
    if [[ $? != 0 ]]; then
        log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db no runnig now."

        # 启动数据库
        restart_to_primary; ret_val=$?
        return $ret_val
    fi

    # 获取数据库状态
    dbinfo=$(get_db_status_info)

    # 从数据库查询结果中分理处各项信息: local_role, db_state, detail_information, peer_role. 前四个变量会在get_db_state赋值
    get_db_state "$dbinfo"; ret_val=$?

    case "$local_role" in
        # 本端pending时，直接升主
        "$gs_r_pending")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db current role[$local_role], current ha role[$RUN_STATE], need restart. [$dbinfo]"
            # pending 数据库BUG 升主前需要翻转数据库配置
            reload_float_ip
            notify_to_primary; ret_val=$?
            return $ret_val
            ;;

        # 已经是主状态，不需要处理
        "$gs_r_primary")
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db current role[$local_role] no need to change. [$dbinfo]"
            return $r_success
            ;;

        "$gs_r_standby")
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db current role[$local_role], need further check. [$dbinfo]"
            ;;

        # 以下角色不是双机会出现的角色，直接重启
        "$gs_r_normal"|"$gs_r_cstandby"|"$gs_r_unknown"|*)
            log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db current role[$local_role] error! [$dbinfo]"
            restart_to_primary; ret_val=$?
            return $ret_val
            ;;
    esac

    # 本端角色不正确，需要判断对端，以及重建情况
    case "$db_state" in
        # 对端在位，且状态正常，先尝试switchover再尝试failover
        "$gs_s_normal")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db state[$db_state] stable now, start change to primary. [$dbinfo]"
            sta_to_pri; ret_val=$?
            return $ret_val
            ;;

        # 以下几种状态，表示数据库状态不稳定，待稳定后进行下一步处理，以避免出现问题。
        "$gs_s_rebuilding"|"$gs_s_starting"|"$gs_s_demoting"|"$gs_s_promoting"|"$gs_s_waiting")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $r_success
            ;;

        # 数据库需要重建，待修复处理。
        "$gs_s_needrepair")
            # 重建状态不一定需要修复的，这个需要高斯DB后续修改。
            if echo "$detail_information" | grep -Ew "$gs_r_canrepair"; then
                log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db state[$db_state] need repair now, can not start to primary. [$dbinfo]"
                return $r_success
            fi

            # 不需要修复的情况，直接跳转到需要的状态
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db state[$db_state], detail_information[$detail_information] can not repair, start change to primary. [$dbinfo]"
            restart_to_primary; ret_val=$?
            return $ret_val
            ;;

        # 其他状态，直接重启跳转到需要的状态
        "$gs_s_unknown"|*)
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_activate] db state[$db_state] stable now, can goto want role. [$dbinfo]"
            restart_to_primary; ret_val=$?
            return $ret_val
            ;;
    esac

    return $r_success
}

######################################################################
#   FUNCTION   : do_deactivate
#   DESCRIPTION: 数据库去激活函数
######################################################################
do_deactivate()
{
    # 本端没有启动，尝试启动为备
    is_db_running
    if [[ $? != 0 ]]; then
        log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db no runnig now."

        # 启动数据库
        restart_to_standby; ret_val=$?
        return $ret_val
    fi

    # 获取数据库状态
    dbinfo=$(get_db_status_info)

    # 从数据库的查询结果中分离出: local_role, db_state, detail_information, peer_role. 前四个变量会在get_db_state赋值。
    get_db_state "$dbinfo"; ret_val=$?

    case "$local_role" in
        # 本段pending直接降备
        "$gs_r_pending")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db current role[$local_role], current ha role[$RUN_STATE], need notify to standby. [$dbinfo]"
            notify_to_standby; ret_val=$?
            return $ret_val
            ;;

        # 已经是备用角色，不需要处理
        "$gs_r_standby")
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db current role[$local_role] no need to change. [$dbinfo]"
            return $r_success
            ;;

        # 主角色下，需要检查
        "$gs_r_primary")
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db current role[$local_role], need further check. [$dbinfo]"
            ;;

        # 以下角色不是双机会出现的角色，直接重启
        "$gs_r_normal"|"$gs_r_cstandby"|"$gs_r_unknown"|*)
            log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db current role[$local_role] error! [$dbinfo]"
            restart_to_standby; ret_val=$?
            return $ret_val
            ;;
    esac

    # 本端角色不正确，需要判断对端，以及重建情况。
    case "$db_state" in
        # 对端在位，且状态正常，不能直接降备，需要等待对端进行swithover或是双主时降备。
        "$gs_s_normal")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db state[$db_state] stable now, start change to stabndby. [$dbinfo]"
            ;;

        # 以下几种状态数据库不稳定，待稳定后再处理。
        "$gs_s_rebuilding"|"$gs_s_starting"|"$gs_s_demoting"|"$gs_s_promoting"|"$gs_s_waiting")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db state[$db_state] instable now, need wait. [$dbinfo]"
            return $r_success
            ;;

        # 数据库需要重建，待修复时处理
        "$gs_s_needrepair")
            # 重建状态不一定都是需要修复的，这个需要高斯DB后修改
            if echo "$detail_information" | grep -Ew "$gs_r_canrepair"; then
                log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db state[$db_state] need repair now, can not start to standby. [$dbinfo]"
                return $r_success
            fi

            # 不需要修复的情况，直接跳转到需要的状态
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db state[$db_state], detail_information[$detail_information] can not repair, start change to stabndby. [$dbinfo]"
            ;;

        # 其他状态，直接重启跳转到需要的状态
        "$gs_s_unknown"|*)
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db state[$db_state] stable now, can goto want role. [$dbinfo]"
            restart_to_standby; ret_val=$?
            return $ret_val
            ;;
    esac

    # 主降备，在操作前，需要看对端的状态
    if [ "" == "$peer_role" ]; then
        pri_to_sta; ret_val=$?
        return $ret_val
    fi

    log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_deactivate] db state[$db_state] stable now, and peer role[$peer_role], need peer to primary first. [$dbinfo]"
    return $r_success
}


######################################################################
#   FUNCTION   : do_notify
#   DESCRIPTION: 数据异常时，通知函数
######################################################################
do_notify()
{
    return $r_success
}


######################################################################
#   FUNCTION   : do_repair
#   DESCRIPTION: 数据修复函数
######################################################################
do_repair()
{
    local -i ret_val=$r_success

    if [ "$gs_cur_mode" == "$gs_db_single" ]; then
        do_repair_single ; ret_val=$?
    else
        do_repair_double ; ret_val=$?
    fi
    return $ret_val
}


######################################################################
#   FUNCTION   : do_repair_single
#   DESCRIPTION: 单机数据修复函数
######################################################################
do_repair_single()
{
    # 数据库角色不正确，在次修复
    # 如果当前角色不正确，则进行重启，但是如果正在处理修复阶段，则先等待。
    local dbinfo=""
    local -i ret_val=$r_success

    # 查询数据库是否正常运行
    # 数据库未启动
    is_db_running
    if [[ $? != 0 ]]; then
        log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_single] db no runnig now."

        # 启动数据库
        do_start_single; ret_val=$?
        return $ret_val
    fi

    # 获取数据库状态
    dbinfo=$(get_db_status_info)

    # 从数据库的查询结果中分离: local_role, db_state, detail_information, peer_role. 前四个变量会在get_db_state被赋值
    get_db_state "$dbinfo"; ret_val=$?

    # 根据数据库的角色进行判断，正常情况下，只会出现Normal和空（即获取失败）
    case "$local_role" in
        "$gs_r_normal")
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "[do_repair_single] db current role Normal. [$dbinfo]"
            return $db_normal
            ;;

        # 以下三种状态，虽然在单机下不可能出现，但是需要进行异常处理，这三种状态下，目前不进行判断，直接重启。
        "$gs_r_primary"|"$gs_r_standby"|"$gs_r_cstandby")
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_single] db current role error[$local_role], need restart. [$dbinfo]"
            # 重启数据库
            restart_to_normal; ret_val=$?
            return $ret_val
            ;;

        # 其他角色不是数据库的正常角色，直接重启数据库
        *)
            log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "[do_repair_single] db current role[$local_role] error! [$dbinfo]"
            # 重启数据库
            restart_to_normal; ret_val=$?
            return $ret_val
            ;;
    esac

    # 重启数据库
    restart_to_normal; ret_val=$?+
    return $ret_val
}

######################################################################
#   FUNCTION   : do_repair_double
#   DESCRIPTION: 双机数据库修复函数
######################################################################
do_repair_double()
{
    # 修复只做rebuild操作，数据库角色不正确通过激活和去激活恢复
    # 修复操作只有数据库角色为备或级联备，同事数据库状态为NeedRepair
    # 而且数据库连接状态不为连接中，或是连接断开。
    local dbinfo=""
    local -i ret_val=$r_success

    # 查询数据库是否正常运行
    # 数据库未启动
    is_db_running
    if [[ $? != 0 ]]; then
        log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_double] db no runnig now."

        # 数据库没有运行，有可能是启动了，检查是否需要重建。
        is_need_rebuild; ret_val=$?
        if [ $ret_val -eq $db_abnormal ]; then
            # 调用命令进行重建
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_double] start to repair by [$OMM_GS_CTL build]. "
            sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_build >>"$LOG_FILE" 2>&1 ; ret_val=$?
            # 重建完成，需要清空记录
            record_start_result $ret_val
            log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_double] repair finish[$ret_val]."
            return $ret_val
        fi

        return $db_stopped
    fi

    # 获取数据库状态
    dbinfo=$(get_db_status_info)

    # 从数据库查询结果中分离: local_role, db_state, detail_information, peer_role. 前四项会子啊get_db_state赋值
    get_db_state "$dbinfo"; ret_val=$?

    # 不为备状态，或是级联备，不处理
    if [ "$local_role" != "$gs_r_standby" -a "$local_role" != "$gs_r_cstandby" ]; then
        log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_double] local_role[$local_role] error. [$dbinfo]"
        return $r_success
    fi

    # 不是待修复状态，不处理
    if [ "$db_state" != "$gs_s_needrepair" ]; then
        log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_double] current db_state[$db_state] no need to repair. [$dbinfo]"
        return $r_success
    fi

    # 不是待修复状态，不处理
    if ! echo "$detail_information" | grep -Ew "$gs_r_canrepair"; then
        log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_double] current detail_information[$detail_information] error, it can not be repair. [$dbinfo]"
        return $r_success
    fi

    log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_double] start exc:[$OMM_GS_CTL build] to repair. [$dbinfo]"
    sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh db_build >>"$LOG_FILE" 2>&1 ; ret_val=$?
    log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_double] Ended to repair, GaussDB return: [$ret_val]."

    # 详细信息不需要重建，则不重建
    record_start_result $ret_val
    log "$WARN" "${FUNCNAME[0]}" "$LINENO" "[do_repair_double] repair finish[$ret_val]."

    return $ret_val
}

######################################################################
#   FUNCTION   : alarm_rpt
#   DESCRIPTION: report alarm
######################################################################
alarm_rpt()
{
    return 0
}

######################################################################
#   FUNCTION   : notify_service
#   DESCRIPTION: notify Service DB
######################################################################
notify_service()
{
    log "$INFO" "${FUNCNAME[0]}" "$LINENO" "Resource Status Notify: Status=$1"

    local -i res_status="$1"
    local ha_node_name="$2"

    case "$res_status" in
    0)
      ##$1: type, e.g.: create, recover, event
      ##$2: event name, e.g.: EVENT_HA_STATUS_EXCEPTION
      ##$3: resource, e.g.: gaussdb, floatIp...
      log "$INFO" "${FUNCNAME[0]}" "$LINENO" "recover alarm EVENT_HA_STATUS_EXCEPTION,resource:gaussdb"
    ;;
    1)
      log "$INFO" "${FUNCNAME[0]}" "$LINENO" "create alarm EVENT_HA_STATUS_EXCEPTION,resource:gaussdb"
    ;;
    *)
      log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "unknow alarm number:[$res_status]."
    ;;
    esac

    return $?
}