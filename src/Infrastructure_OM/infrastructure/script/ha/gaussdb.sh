#!/bin/bash
set +x

. /etc/profile
G_HA_SCRIPT_PATH=/usr/local/ha/script
G_SUDO_SCRIPT_PATH=/opt/script
G_HA_TMP_PATH=/usr/local/ha/local/tmp
G_RESOURCE_NAME="gaussdb"
CLUSTER_ROLE_PRIMARY="PRIMARY"
CLUSTER_ROLE_Standby="STANDBY"
CHANGE_ROLE_FAIL_FILE=${G_HA_TMP_PATH}/change_role
G_NODE_MARK_FILE="/opt/third_data/ha/gaussdb_node"
source ${G_HA_SCRIPT_PATH}/check_service.sh

if ! . "${G_HA_SCRIPT_PATH}/dbcommand.sh" ; then
    log "$ERROR" "${FUNCNAME[0]}" "$LINENO" "load ${G_HA_SCRIPT_PATH}/dbcommand.sh failure!"
    exit 2
fi

####################################################################
#  FUNCTION     : main
#  DESCRIPTION  : PMS服务函数
#  CALLS        : 无
#  CALLED BY    : 无
#  INPUT        : 无
#  OUTPUT       : 无
#  READ GLOBVAR : 无
#  WRITE GLOBVAR: 无
#  RETURN       : 0 是
#                 1 否
###################################################################
function main() {
  local -i ret_val=0
  case "$OPT_COMMAND" in
  status)
    # To Do 主备模式资源会用到$2
    # 查询资源状态，返回码为上面列出来的返回码
    check_status_process ${G_RESOURCE_NAME}
    if [ $? -ne 0 ]; then
      return 2
    fi
    # 更新gaussdb所在控制器至标记文件
    echo $NODE_NAME >$G_NODE_MARK_FILE
    get_status
    ret_val=$?
    return $ret_val
    ;;

    start)
        # To Do 主备模式资源不会用到该action（需要使用active）
        # 启动资源，返回码 0表示成功 1表示失败
        do_start; ret_val=$?
        return $ret_val
        ;;

    stop)
        # To Do
        # 启动资源，返回码 0表示成功 1表示失败
        do_stop; ret_val=$?
        return $ret_val
        ;;

    force-stop)
        # To Do在stop失败时，会执行此操作
        # 强制停止资源，返回码 0表示成功 1表示失败
        do_stop_force; ret_val=$?
        return $ret_val
        ;;

    active)
        # To Do 仅主备模式资源会用到该action
        # 设置ClusterRole为Primary
        sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh change_cluster_role ${CLUSTER_ROLE_PRIMARY}
        if [ $? -ne 0 ]; then
            if [ ! -f "${CHANGE_ROLE_FAIL_FILE}" ]; then
                touch "${CHANGE_ROLE_FAIL_FILE}"
                chmod 660 ${CHANGE_ROLE_FAIL_FILE}
            fi
            echo $CLUSTER_ROLE_PRIMARY > ${CHANGE_ROLE_FAIL_FILE}
        fi
        # 激活资源，返回码 0表示成功 1表示失败
        do_activate; ret_val=$?
	      if [ $ret_val != 0 ];then
	          ##$1: type, e.g.: create, recover, event
            ##$2: event name, e.g.: EVENT_HA_ACTIVE_FAIL
            ##$3:resource, e.g.: gaussdb, floatIp...
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "create alarm EVENT_HA_ACTIVE_FAIL,resource:$G_RESOURCE_NAME"
	      else
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "recover alarm EVENT_HA_ACTIVE_FAIL,resource:$G_RESOURCE_NAME"
	      fi
        return $ret_val
        ;;
    deactive)
        # To Do 仅主备模式资源会用到该Action
        # 设置ClusterRole为Standby
        sudo ${G_SUDO_SCRIPT_PATH}/ha_sudo.sh change_cluster_role ${CLUSTER_ROLE_Standby}
        if [ $? -ne 0 ]; then
            if [ ! -f "${CHANGE_ROLE_FAIL_FILE}" ]; then
                touch "${CHANGE_ROLE_FAIL_FILE}"
                chmod 660 ${CHANGE_ROLE_FAIL_FILE}
            fi
            echo $CLUSTER_ROLE_Standby > ${CHANGE_ROLE_FAIL_FILE}
        fi
        # 去激活资源，返回码 0表示成功 1表示失败
        do_deactivate; ret_val=$?
        if [ $ret_val != 0 ];then
            ##$1: type, e.g.: create, recover, event
            ##$2: event name, e.g.: EVENT_HA_DEACTIVE_FAIL
            ##$3:resource, e.g.: gaussdb, floatIp...
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "create alarm EVENT_HA_DEACTIVE_FAIL,resource:$G_RESOURCE_NAME"
	      else
            log "$INFO" "${FUNCNAME[0]}" "$LINENO" "recover alarm EVENT_HA_DEACTIVE_FAIL,resource:$G_RESOURCE_NAME"
	      fi
        return $ret_val
        ;;

    repair)
        # To Do 此处可能需要使用$2
        # 如果停止失败，则使用force-stop，不需要修复
        # 在start、active、deactive失败时，会调用此接口。
        # 修复资源，返回码 0表示成功 1表示失败
        do_repair; ret_val=$?
        return $ret_val
        ;;
    notify)
        # To Do 此处可能需要使用$2
        # 资源状态变更时，会调用此操作
        # 资源状态变更通知，返回码 0表示成功 1表示失败
        notify_service "$NEXT_STATE" "$HA_NAME"; ret_val=$?
        exit $ret_val
        ;;

    prepare)
        # To Do 此处可能需要用到$2
        #主备切换之前，需要做的一些准备工作
        # ×资源状态变更前的准备工作，返回码 0表示成功 1表示失败
        log "$INFO" "${FUNCNAME[0]}" "$LINENO" "Not support cmd[$@]."
        return $db_notsupported
        ;;

    *)
        #echo "Usage: $0 { stop|status|notify|force-stop|..}"
        # 返回动作不存在
        log "$INFO" "${FUNCNAME[0]}" "$LINENO" "Unknown cmd[$@]."
        return $db_notsupported
        ;;
    esac

}

# 获取脚本入参
in_param_num="$#"
in_param_lst="$@"

declare OPT_COMMAND="$1"; shift                                          # 当前运行的命令
declare RUN_STATE="$1";  shift                                           # 当前ha的运行状态
declare SELF_PARAM="$1"; shift                                           # 自定义参数
declare NEXT_STATE="$1"; shift   # 资源状态变更后的状态（仅在notify时有效，其他情况都无此入参），资源变更后的状态取值为：0正常 1故障
declare HA_NAME="$1";    shift                                           # ha name


declare script_name="$(basename $0)"
log "$WARN" "${FUNCNAME[0]}" "$LINENO" "Enter the script ${script_name}($in_param_num): $0 $in_param_lst"

ha_mode=`get_ha_mode`
if [ "X${ha_mode}" == "Xsingle" ];then
    log "$INFO" "${FUNCNAME[0]}" "$LINENO" "this is single mode"
    gs_cur_mode="$gs_db_single"
else
    log "$INFO" "${FUNCNAME[0]}" "$LINENO" "this is double mode"
    gs_cur_mode="$gs_db_double"
fi

# 导入数据库密码
declare -i script_ret_val=0
main ; script_ret_val=$?

omm_exit $script_ret_val
