#!/bin/sh
#
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
set +x

source "/etc/profile"

AGENT_INSTALL_PATH=${DATA_BACKUP_AGENT_HOME}
 
if [ -z "${AGENT_INSTALL_PATH}" ]; then
    AGENT_INSTALL_PATH="/opt"
fi

if [ ! -d "${AGENT_INSTALL_PATH}" ]; then
    echo "ERROR" "Agent Install Path [${AGENT_INSTALL_PATH}] : No such file or directory."
    return 1
fi

AGENT_ROOT_PATH="${AGENT_INSTALL_PATH}/DataBackup/ProtectClient/ProtectClient-E"
VIRT_ROOT_PATH="${AGENT_INSTALL_PATH}/DataBackup/ProtectClient/Plugins/VirtualizationPlugin"
source "${VIRT_ROOT_PATH}/bin/superlog.sh"

function check_ipv4()
{
    local ipStr="$1"
    local isValid=$(echo ${ipStr}|grep -E "^([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])$"|wc -l);
    if [ ${isValid} -gt 0 ];then
        return 0
    fi

    log_error "ip $ipStr is invalid."
    return 1
}

function check_ipV6()
{
    if echo $1 | grep -E '^((([0-9a-fA-F]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9a-fA-F]{1,4}:){6}((([0-9]{1,3}\.){3}[0-9]{1,4})|([0-9a-fA-F]{1,4})|:))|(([0-9a-fA-F]{1,4}:){5}:((([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:[0-9a-fA-F]{1,4})|(([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){4}:((([0-9a-fA-F]{1,4})?)|(([0-9a-fA-F]{1,4}:){1,2}[0-9a-fA-F]{1,4})|(([0-9a-fA-F]{1,4}:)?([0-9]{1,3}\.){3}[0-9]{1,4})))|(([0-9a-fA-F]{1,4}:){3}:((([0-9a-fA-F]{1,4}:){0,2}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,3}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){2}:((([0-9a-fA-F]{1,4}:){0,3}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,4}[0-9a-fA-F]{1,4})|$))|(([0-9a-fA-F]{1,4}:){1}:((([0-9a-fA-F]{1,4}:){0,4}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,5}[0-9a-fA-F]{1,4})|$))|(::((([0-9a-fA-F]{1,4}:){0,5}([0-9]{1,3}\.){3}[0-9]{1,4})|(([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4})|$)))$' >/dev/null 2>&1;then
        return 0  
    fi
    log_error "ip $1 is invalid."
    return 1
}

function check_ip_vaild()
{
    if [ $# -ne 1 ];then
       return 1
    fi

    echo $1 | grep ':' 1>/dev/null
    if [ $? -ne 0 ];then
        check_ipv4 $1
        return $?
    fi
    check_ipV6 $1
    return $?
}

function check_host_vaild()
{
    if echo $1 | grep -E '^[1-9]\d*|0$' >/dev/null 2>&1;then
        return 0
    fi
    return 1
}

function check_device_vaild()
{
    if echo $1 | grep -E '([1-9]\d*|0)\:([1-9]\d*|0)\:([1-9]\d*|0)\:([1-9]\d*|0)' >/dev/null 2>&1;then
        return 0
    fi
    return 1
}

function check_vir_device_vaild()
{
    if echo $1 | grep -E '^sd[a-z]{1,2}$' >/dev/null 2>&1;then
        return 0
    fi
    return 1
}

function login_iscsi() 
{
    if [ $# -ne 1 ];then
        log_error "Login iscsi params is not valid."
        return 1
    fi
    check_ip_vaild $1
    if [ $? -ne 0 ];then
        log_error "Login iscsi ip $1 is not valid."
        return 1
    fi  

    iscsiadm -m discovery -t st -p $1 >/dev/null 2>&1 && iscsiadm -m node -p $1 -l
    if [ $? -ne 0 ];then
        log_error "Iscsiadm discovery target ip $1 failed."
        return 1
    fi

    return 0
}

function do_uprescan() 
{
    which upRescan >/dev/null 2>&1
    if [ $? -eq 0 ];then
        ps -ef | grep upRescan | grep -v grep
        if [ $? -eq 0 ];then
            log_info "UpRescan is already running."
            return 0
        fi
        timeout 1800 upRescan 2>/dev/null
    else
        log_info "UpRescan is not install, start run rescan-scsi-bus.sh"
        rescan_scsi_bus
        return $?
    fi
    return 0
}

function dev_rescan() 
{
    if [ $# -ne 1 ];then
        log_error "Dev rescan params is not valid."
        return 1
    fi

    check_vir_device_vaild $1
    if [ $? -ne 0 ];then
        log_error "Virtual device $1 is not valid."
        return 1
    fi

    local dev_file="/sys/block/$1/device/rescan"
    if [ ! -f "$dev_file" ];then
        return 1
    fi
    echo 1 > /sys/block/$1/device/rescan
    if [ $? -ne 0 ];then
        log_error "Rescan $1 failed."
        return 1
    fi
    return 0
}

function dev_delete() 
{
    if [ $# -ne 1 ];then
        log_error "Dev delete params is not valid."
        return 1
    fi
    
    check_vir_device_vaild $1
    if [ $? -ne 0 ];then
        log_error "Virtual device $1 is not valid."
        return 1
    fi

    local dev_file="/sys/block/$1/device/delete"
    if [ ! -f "$dev_file" ];then
        return 1
    fi
    echo 1 > /sys/block/$1/device/delete
    if [ $? -ne 0 ];then
        log_error "Dev delete $1 failed."
        return 1
    fi
    log_info "Dev delete $1 success"
    return 0
}

function host_scan() 
{
    if [ $# -ne 1 ];then
        log_error "Host scan params is not valid."
        return 1
    fi

    check_host_vaild $1
    if [ $? -ne 0 ];then
        log_error "Host $1 is not valid."
        return 1
    fi

    local host_file="/sys/class/scsi_host/host$1/scan"
    if [ ! -f "$host_file" ];then
        return 1
    fi 
    echo "- - -" > "$host_file"
    if [ $? -ne 0 ];then
        log_error "Rescan host$1 failed."
        return 1
    fi
    return 0
}

function scsi_delete() 
{
    if [ $# -ne 1 ];then
        log_error "Delete device params is not valid."
        return 1
    fi

    check_device_vaild $1
    if [ $? -ne 0 ];then
        log_error "Delete device $1 is not valid."
        return 1
    fi

    local scsi_file="/sys/class/scsi_device/$1/device/delete"
    if [ ! -f "$scsi_file" ];then
        return 1
    fi 
    echo 1 > "${scsi_file}"
    if [ $? -ne 0 ];then
        log_error "Delete scsi device $1 failed."
        return 1
    fi
    return 0
}

function get_fc_port_name() 
{
    cat /sys/class/fc_host/host*/port_name
    if [ $? -ne 0 ];then
        log_error "Get fc port name failed."
        return 1
    fi
    return 0
}

function get_iscsi_hostNumber() 
{
    iscsiadm -m session -P 3 | grep 'Host Number' | awk -F ':' '{print $2}' | awk '{print $1}'
    if [ $? -ne 0 ];then
        log_error "Get iscsi host number failed."
        return 1
    fi
    return 0
}

function do_multipath() 
{
    multipath -ll >/dev/null 2>&1
    if [ $? -ne 0 ];then
        log_error "Run multipath failed."
        return 1
    fi
    return 0
}

function rescan_scsi_bus() 
{
    rescan-scsi-bus.sh -r >/dev/null 2>&1 && rescan-scsi-bus.sh -r >/dev/null 2>&1
    if [ $? -ne 0 ];then
        log_error "Run rescan-scsi-bus.sh failed."
        return 1
    fi
    log_info "Run rescan-scsi-bus.sh finished."
    return 0
}

function rescan_iscsi_session(){
    iscsiadm -m session --rescan rescan-scsi-bus.sh
    if [ $? -ne 0 ];then
        log_error "Rescan iscsi session failed."
        return 1
    fi
    log_info "Rescan iscsi session finished."
    return 0
}

function get_host_uuid()
{
    dmidecode -s system-uuid
    if [ $? -ne 0 ]; then
        log_error "Get host uuid failed."
        return 1
    fi
    return 0
}

function get_host_sn()
{
    cat /etc/HostSN/HostSN
    if [ $? -ne 0 ]; then
        log_error "Get host sn."
        return 1
    fi
    return 0
}

function get_all_disk_letter()
{
    lsblk -d -o name | grep -v '[0-9]'
    if [ $? -ne 0 ]; then
        log_error "Get all disk letters failed."
        return 1
    fi
    log_info "Get all disk letters success"
    return 0
}

function get_disk_wwn_by_wwid()
{
    cat /sys/class/block/$1/device/wwid
    if [ $? -ne 0 ]; then
        log_error "Get disk wwn by wwid failed."
        return 1
    fi
    log_info "Get disk wwn by wwid success."
    return 0
}

function check_iscsi_exist()
{
    iscsiadm --help
    if [ $? -ne 0 ]; then
        log_error "Get iscsiadm failed."
        return 1
    fi
    return 0
}

function get_iscsi_iqn()
{
    cat /etc/iscsi/initiatorname.iscsi | grep InitiatorName= | awk -F '=' '{print $2}'
    if [ $? -ne 0 ]; then
        log_error "Get iscsi iqn failed."
        return 1
    fi
    return 0
}

function discovery_iscsi_target()
{
    if [ $# -ne 1 ]; then
        log_error "Iscsiadm discovery target ip params is not valid."
        return 1
    fi

    iscsiadm -m discovery -t st -p $1
    if [ $? -ne 0 ]; then
        log_error "Iscsiadm discovery target ip $1 failed."
        return 1
    fi

    return 0
}

function login_iscsi_target()
{
    if [ $# -ne 2 ]; then
        log_error "Iscsiadm login target ip params is not valid."
        return 1
    fi

    iscsiadm -m node -T $1 -p $2 -l
    if [ $? -ne 0 ];then
        log_error "Iscsiadm login target ip $2 failed."
        return 1
    fi

    return 0
}

function logout_iscsi_target()
{
    iscsiadm -m node -p $1 --logout
    if [ $? -ne 0 ];then
        log_error "Iscsiadm logout target ip $1 failed."
        return 1
    fi

    return 0
}

function get_fc_initor()
{
    cat /sys/class/fc_host/host*/port_name
    if [ $? -ne 0 ];then
        log_warn "Get fc initor failed."
        return 1
    fi
    return 0
}

function get_loggedIn_targetIp()
{
    iscsiadm -m  session -P 1 | awk -F ':' '/Target/{for (i=7;i<=NF;i++)print($i);}' | sed 's/.$//' | sed 's/.(.*$//'
    if [ $? -ne 0 ]; then
        log_warn "Get logged in target ip failed."
        return 1
    fi
    return 0
}

function get_loggedIn_nodeIp()
{
    iscsiadm -m node | awk -F ':' '{for (i=1;i<=1;i++)print($i);}'
    if [ $? -ne 0 ]; then
        log_warn "Get logged in node ip failed."
        return 1
    fi
    return 0
}

function get_scsi_list()
{
    lsscsi
    if [ $? -ne 0 ]; then
        log_error "Get scsi list failed."
        return 1
    fi
    return 0
}

function check_upRescan()
{
    which upRescan >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        log_error "Not found upRescan tools."
        return 1
    fi
    return 0
}

function get_diskpath_for_wwn()
{
    if [ $# -ne 1 ]; then
        log_error "Get diskpath for wwn params is not valid."
        return 1
    fi

    ls -al /dev/disk/by-id | grep $1 | grep -v "part" | grep "scsi-"
    if [ $? -ne 0 ]; then
        log_error "Get diskpath for wwn ($1) failed."
        return 1
    fi

    return 0
}

function get_diskpath_list()
{
    ls -al /dev/disk/by-id | grep -v "part" | grep "scsi-"
    if [ $? -ne 0 ]; then
        log_error "Get diskpath list failed."
        return 1
    fi

    return 0
}

function check_ip_connect()
{
    if [ $# -ne 1 ];then
        log_error "Check ip connect params is not valid."
        return 1
    fi
    check_ip_vaild $1
    if [ $? -ne 0 ];then
        log_error "Check ip connect, ip $1 is not valid."
        return 1
    fi
    ip=$1
    sysName=`uname -s`
    if [ $sysName = "Linux" ]; then
        su - rdadmin -s /bin/sh -l -c "${AGENT_ROOT_PATH}/bin/agentcli testhost ${ip} 3260 200">> $LOG_SCRIPT_FILE 2>&1
    else
        su - rdadmin -c "${AGENT_ROOT_PATH}/bin/agentcli testhost ${ip} 3260 200">> $LOG_SCRIPT_FILE 2>&1
    fi
    if [ $? -ne 0 ]; then
        log_error "Check ip connect failed, ${ip} can't access."
        return 1
    fi
    log_debug "Check ip (${ip}) connect success."
    return 0
}

function get_virtio_diskpath_list()
{
    ls -al /dev/disk/by-id/virtio-*
    if [ $? -ne 0 ]; then
        log_error "ERROR: get virtio diskpsth list failed."
        return 1
    fi
    return 0
}

function get_server_uuid()
{
    /usr/sbin/dmidecode | grep UUID | awk -F " " '{print tolower($2)}'
    if [ $? -ne 0 ]; then
        log_error "ERROR: get server uuid failed."
        return 1
    fi
    return 0
}

function get_apasara_instance_uuid()
{
    ls -al /var/lib/cloud/instances | grep "i-"  | awk -F "i-" '{print tolower($2)}'
    return 0
}

function get_openstack_cps_ip()
{
    cat /etc/hosts | grep -i identity | awk -F ' ' '{print $1}'
    if [ $? -ne 0 ]; then
        log_error "ERROR: get openstack cps ip."
        return 1
    fi
    return 0
}

function change_priviledge()
{
    chmod $1 $2
    if [ $? -ne 0 ]; then
        log_error "ERROR: change file priviledge."
        return 1
    fi
    return 0
}

function scan_dev()
{
    lsblk | grep disk | awk '{print $1}'
    if [ $? -ne 0 ]; then
        log_error "ERROR: scan dev."
        return 1
    fi
    return 0
}

function update_disk_dev()
{
    partprobe
    if [ $? -ne 0 ]; then
        log_error "ERROR: update disk dev failed."
        return 1
    fi
    return 0
}

function main()
{
    verify_special_char "$@"

    type=$1
    case "$type" in
        login_iscsi)
            shift 1 && login_iscsi "$@"
            return $?
            ;;
        do_uprescan)
            do_uprescan
            return $?
            ;;
        dev_rescan)
            shift 1 && dev_rescan "$@"
            return $?
            ;;
        dev_delete)
            shift 1 && dev_delete "$@"
            return $?    
            ;;
        host_scan)
            shift 1 && host_scan "$@"
            return $?
            ;;
        scsi_delete)
            shift 1 && scsi_delete "$@"
            return $?    
            ;;
        get_hostNumber)
            get_iscsi_hostNumber
            return $?      
            ;;
        get_fc_port_name)
            get_fc_port_name
            return $?      
            ;;
        do_multipath)
            do_multipath
            return $?   
            ;;	
        rescan_scsi_bus)
            rescan_scsi_bus
            return $?  
            ;;
        rescan_iscsi_session)
            rescan_iscsi_session
            return $?  
            ;;
        get_host_uuid)
            get_host_uuid
            return $?
            ;;
        get_host_sn)
            get_host_sn
            return $?
            ;;
        get_all_disk_letter)
            get_all_disk_letter
            return $?
            ;;
        get_disk_wwn_by_wwid)
            shift 1 && get_disk_wwn_by_wwid "$@"
            return $?
            ;;
        check_iscsi_exist)
          	check_iscsi_exist
          	return $?
          	;;
        get_iscsi_iqn)
            get_iscsi_iqn
            return $?
            ;;
        discovery_iscsi_target)
            shift 1 && discovery_iscsi_target "$@"
            return $?
            ;;
        login_iscsi_target)
            shift 1 && login_iscsi_target "$@"
            return $?
            ;;
        logout_iscsi_target)
            shift 1 && logout_iscsi_target "$@"
            return $?
            ;;
        get_fc_initor)
            get_fc_initor
            return $?
            ;;
        get_loggedIn_targetIp)
            get_loggedIn_targetIp
            return $?
            ;;
        get_loggedIn_nodeIp)
            get_loggedIn_nodeIp
            return $?
            ;;
        get_scsi_list)
            get_scsi_list
            return $?
            ;;
        check_upRescan)
            check_upRescan
            return $?
            ;;
        get_diskpath_for_wwn)
            shift 1 && get_diskpath_for_wwn "$@"
            return $?
            ;;
        get_diskpath_list)
            get_diskpath_list
            return $?
            ;;
        check_ip_connect)
            shift 1 && check_ip_connect "$@"
            return $?
            ;;
        get_virtio_diskpath_list)
            get_virtio_diskpath_list
            return $?
            ;;
        get_server_uuid)
            get_server_uuid
            return $?
            ;;
        get_apasara_instance_uuid)
            get_apasara_instance_uuid
            return $?
            ;;
        get_openstack_cps_ip)
            get_openstack_cps_ip
            return $?
            ;;
        change_priviledge)
            shift 1 && change_priviledge "$@"
            ;;
        scan_dev)
            scan_dev
            return $?
            ;;
        update_disk_dev)
            update_disk_dev
            return $?
            ;;
        *)
        echo "not support $1"
        return 1
            ;;
    esac
}
main "$@"
exit $?
