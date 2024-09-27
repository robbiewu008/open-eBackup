#!/bin/bash
set +x
G_ADD_ROUTE_FILE_PATH=""
G_OS_TYPE="" #操作系统的类型
G_POLICY_ROUTE_START_NUMBER=111


function check_ip()
{
    local dpc_ip=$1
    if [[ $dpc_ip =~ ^([1-9][0-9]?|1[0-9]{2}|2([0-4][0-9]|5[0-5]))(.(([1-9]?|1[0-9])[0-9]|2([0-4][0-9]|5[0-5]))){3}$ ]];then
        return 0
    else
        echo "$dpc_ip is not a valid ip address."
        return 1
    fi
}


function del_if_rule_exist()
{
    local table=$1
    ip rule list |grep $table |awk -F ":" '{print $1}'|while read line
    do
        for num in $line
        do
            ip rule del prio $num
        done
    done
}

function add_route_table()
{
    local storage_ip=$1
    local policy_route_flag=`echo "$storage_ip" | tr '.' '_'`
    policy_route_flag="dpc_storage_${policy_route_flag}"

    echo -e "\e[1;34m Begin add route table to /etc/iproute2/rt_tables. \e[0m"
    line=`nl /etc/iproute2/rt_tables |sed -rn "/${policy_route_flag}/p"|awk -F ' ' '{print $1}' |sed -n '1p'`
    if [ ! -z $line ];then
        return 0
    fi
    
    local route_table_number=""
    route_table_end_number=`cat /etc/iproute2/rt_tables | grep 'dpc_storage' | tail -n 1 | awk '{print $1}'`
    if [ "${route_table_end_number}" = "" ]
    then
        route_table_number="${G_POLICY_ROUTE_START_NUMBER}"
    else
        route_table_number=`expr $route_table_end_number + 1`
    fi
    
    echo "${route_table_number} ${policy_route_flag}" >> /etc/iproute2/rt_tables
    cat /etc/iproute2/rt_tables | grep "${policy_route_flag}" > /dev/null 2>&1
    if [ $? -eq 0 ];then
        echo -e "\e[1;34m Add ${storage_ip} route table to /etc/iproute2/rt_tables succ. \e[0m"
    else
        echo -e "\e[1;31m Add ${storage_ip} route table to /etc/iproute2/rt_tables failed. \e[0m"
        return 1
    fi
    return 0
}

function route_policy()
{
    local storage_ip=$1
    local gateway_ip=$2
    #eBackup-StorageVlan
    local policy_route_flag=`echo "$storage_ip" | tr '.' '_'`
    policy_route_flag="dpc_storage_${policy_route_flag}"
    local netcard=`ifconfig | grep -B 1 "${storage_ip}"| awk -F ':' '{print $1}'| sed -n 1p`
    

    del_if_rule_exist ${policy_route_flag}
    ip rule add from ${storage_ip} table ${policy_route_flag}
	echo "ip rule add from ${storage_ip} table ${policy_route_flag}" >> ${G_BOOT_FILE_PATH}
    if [ "$gateway_ip" != "" ]
    then
		ip route del default via $gateway_ip dev $netcard table ${policy_route_flag} 2>/dev/null
        ip route add default via $gateway_ip dev $netcard table ${policy_route_flag}
        if [ $? -eq 0 ]
        then
			sed -i "/dev $netcard table ${policy_route_flag}/d" ${G_ADD_ROUTE_FILE_PATH}
            echo "ip route add default via $gateway_ip dev $netcard table ${policy_route_flag}" >> ${G_ADD_ROUTE_FILE_PATH}
        else
            echo "ip route add default via $gateway_ip dev $netcard table ${policy_route_flag} exec failed."
            return 1
        fi
    else
		ip route del 0.0.0.0/0 dev $netcard table ${policy_route_flag} 2>/dev/null
        ip route add 0.0.0.0/0 dev $netcard table ${policy_route_flag}
        if [ $? -eq 0 ]
        then
			sed -i "/dev $netcard table ${policy_route_flag}/d" ${G_ADD_ROUTE_FILE_PATH}
            echo "ip route add 0.0.0.0/0 dev $netcard table ${policy_route_flag}" >> ${G_ADD_ROUTE_FILE_PATH}
        else
            echo "ip route add 0.0.0.0/0 dev $netcard table ${policy_route_flag} exec failed."
            return 1
        fi
    fi

}

function add_route()
{
    cp -arfP --remove-destination /etc/iproute2/rt_tables /etc/iproute2/rt_tables.bk
    add_route_table $1
    if [ $? -ne 0 ]
    then
        return 1
    fi
    cp -arfP --remove-destination ${G_ADD_ROUTE_FILE_PATH} ${G_ADD_ROUTE_FILE_PATH}.bk

    route_policy $1 $2
    if [ $? -ne 0 ]
    then
        return 1
    fi

    return 0
}

function main()
{
    ipaddress="$1"
    gateway="$2"
    check_ip $ipaddress
    if [ $? -ne 0 ];then
        echo "Check ip address failed."
        return 1
    fi
    
    if [ "$gateway" != "" ]
    then
        check_ip $gateway
        if [ $? -ne 0 ];then
            echo "Check gateway address failed."
            return 1
        fi
    fi

	G_ADD_ROUTE_FILE_PATH="/sbin/ifup-local"
    G_BOOT_FILE_PATH="/etc/rc.d/rc.local"

    add_route ${ipaddress} ${gateway}
    if [ $? -ne 0 ];then
        echo "Add policy route failed, ip:${ipaddress}, gateway:${gateway}"
        return 1
    fi
}

main $*
exit $?
