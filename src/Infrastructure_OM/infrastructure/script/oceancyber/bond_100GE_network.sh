#!/usr/bin/bash

network_dir="/etc/sysconfig/network-scripts"
service_bond1="eth3"

# 检查是否有100GE网卡
function check_100GE_is_exist() {
    num=`lspci | grep 0222 | wc -l`
    echo "100GE netcard num is ${num}"
    if [ ${num} -eq 0 ]; then
        echo "Error reason: 100GE netcard is not exist."
        exit 1
    fi
}

# 检查eth3是否绑定
function check_service_bond() {
    res=`ls /proc/net/bonding/ | tr '
        ' ' '`
    echo "already bond names: ${res}"
    if [[ ${res} =~ ${service_bond1} ]]; then
        echo "bond error, reason: ${service_bond1} is exist！"
        exit 1
    else
        echo "bond: ${service_bond1} is available."
    fi
}

# 获取所有物理网卡名
function get_100_GE_network_name() {
    IFS=$'
    ' read -d '' -r -a array <<< "$(ls /sys/class/net/ | grep -v "`ls /sys/devices/virtual/net/`" |sed -n '1!p' |  awk -F'\n' '{print $1}')"

    # 查找100GE网卡
    netcard_100GE=""
    for element in "${array[@]}"
    do
        net_card_speed=$(ethtool ${element} | grep "Speed:" | awk -F':' '{print $2}')
        net_card_link_status=$(ethtool ${element} | grep "Link detected:" | awk -F':' '{print $2}')
        # 100GE网卡速率为100000Mb/s,并且连接状态为yes
        if [ ${net_card_speed} ==  "100000Mb/s" ] && [ ${net_card_link_status} ==  "yes" ]; then
            echo "---------------Linked and 100GE name: ${element}-------------"
            netcard_100GE="${netcard_100GE}${element} "
        fi
    done
    echo ${netcard_100GE}
    # 只支持一张100GE网卡
    netcard_100GE_num=$(echo ${netcard_100GE}  | wc -w)
    if [ ${netcard_100GE_num} -ne 2 ]; then
        echo "Error reason: netcard 100GE num is ${netcard_100GE_num}"
        exit 1
    else
        echo "get 100GE netcard names successfully."
    fi

    network_name1=$(echo ${netcard_100GE} | awk -F' ' '{print $1}')
    network_name2=$(echo ${netcard_100GE} | awk -F' ' '{print $2}')
}


function start_network() {
    network_name=$1
    echo "ifup ${network_name}"
    ifup_res=$(ifup ${network_name})
    res_status=$(echo ${ifup_res}|awk '{print $2}')
    if [ $? -ne 0 ] || [ "${res_status}" != "successfully" ]; then
        echo "Faied to start ifup bond: ${network_name}"
        exit 1
    else
        echo "successfully to start ifup bond: ${network_name}"
    fi
}

# 启动eth3网卡
function start_up_network() {
    start_network ${service_bond1}
    sleep 3
    start_network ${network_name1}
    sleep 1
    start_network ${network_name2}
}

# 检查绑定结果
function check_bond_result() {
    bond_result1=$(ip a | grep ${network_name1} | grep ${service_bond1} | wc -l)
    bond_result2=$(ip a | grep ${network_name2} | grep ${service_bond1} | wc -l)
    if [[ ${bond_result1} -ne 1 || ${bond_result2} -ne 1 ]]; then
        echo "Bond failed !"
    else
        echo "Bond successfully."
    fi
}

# 端口组bond
function config_network_bond() {
    inner_nic1=$1
    inner_nic2=$2
    bondname=$3
    if [ ! -f "${network_dir}/ifcfg-${inner_nic1}" ] || [ ! -f "${network_dir}/ifcfg-${inner_nic2}" ]; then
        echo "The network configuration file of ${inner_nic1} or ${inner_nic2} does not exist."
        # 不存在创建
        echo "create ${inner_nic1} configuration file"
        touch ${network_dir}/ifcfg-${inner_nic1}
        echo "create ${inner_nic2} configuration file"
        touch ${network_dir}/ifcfg-${inner_nic2}
    else
        # 存在配置文件就删除
        echo "delete bond connections"
        nmcli con  delete $(nmcli con | grep -E '${service_bond1}|${network_name1}|${network_name2}' | awk '{print $3}')
        echo "shutdown networks"
        ifdown ${service_bond1}
        ifdown ${network_name1}
        ifdown ${network_name2}
        echo "delete ifcfg files"
        rm -rf ${network_dir}/ifcfg-${bondname}
        rm -rf ${network_dir}/ifcfg-${inner_nic1}
        rm -rf ${network_dir}/ifcfg-${inner_nic2}
    fi

    file_list=("${inner_nic1}" "${inner_nic2}")
    for file in ${file_list[*]}
    do
        network_file="${network_dir}/ifcfg-${file}"
        sed -i '1,$d' ${network_file}
        echo "DEVICE=${file}" >> ${network_file}
        echo "ONBOOT=yes" >> ${network_file}
        echo "BOOTPROTO=none" >> ${network_file}
        echo "STARTMODE=auto" >> ${network_file}
        echo "USERCTL=no" >> ${network_file}
        echo "SLAVE=yes" >> ${network_file}
        echo "MASTER=${bondname}" >> ${network_file}
    done

    network_file_bond="${network_dir}/ifcfg-${bondname}"
    if [ -f ${network_file_bond} ];then
        rm ${network_file_bond}
    fi
    touch ${network_file_bond}
    echo "DEVICE=${bondname}" >> ${network_file_bond}
    echo "ONBOOT=yes" >> ${network_file_bond}
    echo "BONDING_MASTER=yes" >> ${network_file_bond}
    echo "BOOTPROTO=static" >> ${network_file_bond}
    echo "STARTMODE=auto" >> ${network_file_bond}
    echo "BONDING_OPTS='mode=1 miimon=200'" >> ${network_file_bond}
}


function main()
{
    echo "check 100GE card is exist"
    check_100GE_is_exist

    echo "start check service bond"
    check_service_bond

    echo "start get 100GE network name"
    get_100_GE_network_name

    echo "start to bond service port"
    config_network_bond ${network_name1} ${network_name2} ${service_bond1}

    echo "start up eth3 bond"
    start_up_network

    echo "start to check bond result"
    check_bond_result
}

main
