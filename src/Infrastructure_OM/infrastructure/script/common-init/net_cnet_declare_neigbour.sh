#!/bin/bash

declare -A g_cnetIpv4Map
declare -A g_cnetIpv4TimeMap1
declare -A g_cnetIpv4TimeMap2
declare -A g_cnetIpv4TimeMap3
declare -A g_cnetIpv4SendArpFlag1
declare -A g_cnetIpv4SendArpFlag2
declare -A g_cnetIpv4SendArpFlag3
declare -A g_cnetIpv4OldMap
declare -A g_cnetIpv4TimeOldMap1
declare -A g_cnetIpv4TimeOldMap2
declare -A g_cnetIpv4TimeOldMap3
declare -A g_cnetIpv6Map
declare -A g_cnetIpv6TimeMap1
declare -A g_cnetIpv6TimeMap2
declare -A g_cnetIpv6TimeMap3
declare -A g_cnetIpv6OldMap
declare -A g_cnetIpv6TimeOldMap1
declare -A g_cnetIpv6TimeOldMap2
declare -A g_cnetIpv6TimeOldMap3
declare -A g_cnetIpv6SendArpFlag1
declare -A g_cnetIpv6SendArpFlag2
declare -A g_cnetIpv6SendArpFlag3

NET_ARP_NO_SEND_PKT_FLAG=0
NET_ARP_SEND_PKT_FLAG=1

function rand()
{
    local min=$1
    local max=$(($2-$min+1))
    local num=$(date +%s%N)
    echo $(($num%$max+$min))
}

function CnetGetSysTime()
{
    local sysTime=$(cat /proc/uptime | awk '{print $1}')
    local int=$(echo $sysTime | awk -F. '{print $1}')
    local dec=$(echo $sysTime | awk -F. '{print $2}')
    
    sysTime=$((10#${dec}+${int}*100))
    
    echo ${sysTime}
}

function CnetParseIpv4()
{
    local i=0
    local ip=''
    local array=(`ip -4 addr | grep -E "^ +inet " | grep -v  "inet 127." | awk -F' ' '{print $2,$NF}'`)
    local array_len=${#array[@]}
    local sysTime=$(CnetGetSysTime)

    while [ $i -lt $array_len ]; do
        ip=${array[$i]%/*}
        g_cnetIpv4Map[${ip}]=${array[(($i+1))]}
        g_cnetIpv4TimeMap1[${ip}]=$(($(rand 0 555)+$sysTime+500))
        g_cnetIpv4TimeMap2[${ip}]=$(($(rand 0 555)+$sysTime+3000))
        g_cnetIpv4TimeMap3[${ip}]=$(($(rand 0 555)+$sysTime+6000))
        let i+=2
    done
    return
} 

function CnetDoArping()
{
    local key=$1
    arping -c 1 -w 1 -Uq -I "${g_cnetIpv4Map[$key]}" -s "$key" "$key"
    echo  "[CNET] arping -c 1 -w 1 -Uq -I ${g_cnetIpv4Map[$key]} -s $key $key, ret $?."
    return
}

function CnetRepeatSendIpv4Arp()
{
    local key=$1
    local i
    local sysTime=$(CnetGetSysTime)
    local arrayFlag=(${g_cnetIpv4SendArpFlag1[$key]} ${g_cnetIpv4SendArpFlag2[$key]} ${g_cnetIpv4SendArpFlag3[$key]})
    local arrayTimeMap=(${g_cnetIpv4TimeOldMap1[$key]} ${g_cnetIpv4TimeOldMap2[$key]} ${g_cnetIpv4TimeOldMap3[$key]})
    
    for ((i=2;i>=0;i--)); do
        if [[ ${arrayFlag[$i]} -eq ${NET_ARP_SEND_PKT_FLAG} ]]; then
            return
        fi
        if [[ ${arrayTimeMap[$i]} -le ${sysTime} ]]; then
            CnetDoArping "${key}"
            case $i in
                0)
                    g_cnetIpv4SendArpFlag1[$key]=${NET_ARP_SEND_PKT_FLAG}
                ;;
                1)
                    g_cnetIpv4SendArpFlag2[$key]=${NET_ARP_SEND_PKT_FLAG}
                ;;
                2)
                    g_cnetIpv4SendArpFlag3[$key]=${NET_ARP_SEND_PKT_FLAG}
                ;;
            esac
            return
        fi
    done
    
    return
}

function CnetSendIpv4Arp()
{
    local key=""
    
    for key in ${!g_cnetIpv4Map[@]}; do
        if [ "${g_cnetIpv4Map[$key]}" == "${g_cnetIpv4OldMap[$key]}" ]; then
            CnetRepeatSendIpv4Arp "$key"
            continue    
        fi
        CnetDoArping "$key"
    done
    return
}

function CnetUnsetIpv4Map()
{
    local key=$1
    
    unset g_cnetIpv4Map[${key}]
    unset g_cnetIpv4TimeMap1[${key}]
    unset g_cnetIpv4TimeMap2[${key}]
    unset g_cnetIpv4TimeMap3[${key}]
    return
}

function CnetUnsetIpv4OldMap()
{
    local key=$1
    
    unset g_cnetIpv4OldMap[${key}]
    unset g_cnetIpv4TimeOldMap1[${key}]
    unset g_cnetIpv4TimeOldMap2[${key}]
    unset g_cnetIpv4TimeOldMap3[${key}]
    unset g_cnetIpv4SendArpFlag1[${key}]
    unset g_cnetIpv4SendArpFlag2[${key}]
    unset g_cnetIpv4SendArpFlag3[${key}]
    return
}

function CnetSetIpv4Map()
{
    local key=$1
    
    g_cnetIpv4OldMap[${key}]=${g_cnetIpv4Map[${key}]}
    g_cnetIpv4TimeOldMap1[${key}]=${g_cnetIpv4TimeMap1[${key}]}
    g_cnetIpv4TimeOldMap2[${key}]=${g_cnetIpv4TimeMap2[${key}]}
    g_cnetIpv4TimeOldMap3[${key}]=${g_cnetIpv4TimeMap3[${key}]}
    g_cnetIpv4SendArpFlag1[${key}]=${NET_ARP_NO_SEND_PKT_FLAG}
    g_cnetIpv4SendArpFlag2[${key}]=${NET_ARP_NO_SEND_PKT_FLAG}
    g_cnetIpv4SendArpFlag3[${key}]=${NET_ARP_NO_SEND_PKT_FLAG}
    return
}

function CnetRefreshIpv4()
{
    local key=""
    
    for key in ${!g_cnetIpv4OldMap[@]}; do
        if [ "${g_cnetIpv4Map[$key]}" != "${g_cnetIpv4OldMap[$key]}" ];then
            CnetUnsetIpv4OldMap "${key}"
        fi        
    done
    for key in ${!g_cnetIpv4Map[@]}; do
        if [ "${g_cnetIpv4Map[$key]}" == "${g_cnetIpv4OldMap[$key]}" ];then
            CnetUnsetIpv4Map "${key}"
            continue
        fi
        CnetSetIpv4Map "${key}"
        CnetUnsetIpv4Map "${key}"
    done
}

function CnetUnsetIpv6Map()
{
    local key=$1
    
    unset g_cnetIpv6Map[${key}]
    unset g_cnetIpv6TimeMap1[${key}]
    unset g_cnetIpv6TimeMap2[${key}]
    unset g_cnetIpv6TimeMap3[${key}]
    return
}

function CnetUnsetIpv6OldMap()
{
    local key=$1
    
    unset g_cnetIpv6OldMap[${key}]
    unset g_cnetIpv6TimeOldMap1[${key}]
    unset g_cnetIpv6TimeOldMap2[${key}]
    unset g_cnetIpv6TimeOldMap3[${key}]
    unset g_cnetIpv6SendArpFlag1[${key}]
    unset g_cnetIpv6SendArpFlag2[${key}]
    unset g_cnetIpv6SendArpFlag3[${key}]
    return
}

function CnetSetIpv6Map()
{
    local key=$1
    
    g_cnetIpv6OldMap[${key}]=${g_cnetIpv6Map[${key}]}
    g_cnetIpv6TimeOldMap1[${key}]=${g_cnetIpv6TimeMap1[${key}]}
    g_cnetIpv6TimeOldMap2[${key}]=${g_cnetIpv6TimeMap2[${key}]}
    g_cnetIpv6TimeOldMap3[${key}]=${g_cnetIpv6TimeMap3[${key}]}
    g_cnetIpv6SendArpFlag1[${key}]=${NET_ARP_NO_SEND_PKT_FLAG}
    g_cnetIpv6SendArpFlag2[${key}]=${NET_ARP_NO_SEND_PKT_FLAG}
    g_cnetIpv6SendArpFlag3[${key}]=${NET_ARP_NO_SEND_PKT_FLAG}
    return;
}

function CnetIsDrvName()
{
    local drvName=$1
    local ret=`echo $drvName | grep -E "eth|bri|@"`
    if [ "x$ret" != "x" ]; then
        return 1
    fi
    return 0
}

function CnetIsIpv6()
{
    local ipv6=$1
    local ret=`echo $ipv6 | grep "[^0-9a-fA-F:/]"`
    if [ "x$ret" = "x" ]; then
        return 1
    fi
    return 0
}

function CnetGetIpv6()
{
    local ret=0
    local ip6=''
    local i=$3
    local array=($1)
    local array_len=$2
    local drvName=$4
    local sysTime=$(CnetGetSysTime)
    
    while [ $i -lt $array_len ]; do
        CnetIsIpv6 ${array[$i]}
        ret=$?
        if [ $ret -ne 0 ]; then
            drvName="${drvName%@*}"
            ip6=${array[$i]%/*}
            g_cnetIpv6Map[${ip6}]="${drvName%:}"
            g_cnetIpv6TimeMap1[${ip6}]=$(($(rand 0 555)+$sysTime+500))
            g_cnetIpv6TimeMap2[${ip6}]=$(($(rand 0 555)+$sysTime+3000))
            g_cnetIpv6TimeMap3[${ip6}]=$(($(rand 0 555)+$sysTime+6000))
            let i+=1
            continue
        fi
        return $i
    done
    return $i
}

function CnetParseIpv6()
{
    local i=0
    local j=0
    local ret=0
    local drvName=""
    local array=(`ip -6 addr | grep -E "[0-9]+:| inet6 " | awk '{print $2}'`)
    local array_len=${#array[@]}
    
    while [ $i -lt $array_len ]; do
        drvName=${array[$i]}
        CnetIsDrvName $drvName
        ret=$?
        let i+=1
        if [ $ret -ne 0 ]; then
            CnetGetIpv6 "${array[*]}" $array_len $i $drvName
            i=$?
        fi
    done
    return
}

function CnetDoNd()
{
    local key=$1
    
    ndsend "$key" "${g_cnetIpv6Map[$key]}"
    echo "[CNET]ndsend $key ${g_cnetIpv6Map[$key]}, ret $?."
    return
}

function CnetRepeatSendIpv6Nd()
{
    local key=$1
    local i=0
    local sysTime=$(CnetGetSysTime)
    local arrayFlag=(${g_cnetIpv6SendArpFlag1[$key]} ${g_cnetIpv6SendArpFlag2[$key]} ${g_cnetIpv6SendArpFlag3[$key]})
    local arrayTimeMap=(${g_cnetIpv6TimeOldMap1[$key]} ${g_cnetIpv6TimeOldMap2[$key]} ${g_cnetIpv6TimeOldMap3[$key]})
    
    for ((i=2;i>=0;i--)); do
        if [[ ${arrayFlag[$i]} -eq ${NET_ARP_SEND_PKT_FLAG} ]]; then
            return
        fi
        if [[ ${arrayTimeMap[$i]} -le ${sysTime} ]]; then
            CnetDoNd "${key}"
            case $i in
                0)
                    g_cnetIpv6SendArpFlag1[$key]=${NET_ARP_SEND_PKT_FLAG}
                ;;
                1)
                    g_cnetIpv6SendArpFlag2[$key]=${NET_ARP_SEND_PKT_FLAG}
                ;;
                2)
                    g_cnetIpv6SendArpFlag3[$key]=${NET_ARP_SEND_PKT_FLAG}
                ;;
            esac
            return
        fi
    done
    
    return
}

function CnetSendIpv6Nd()
{
    local key=""
    
    for key in ${!g_cnetIpv6Map[@]}; do
        if [ "${g_cnetIpv6Map[$key]}" = "${g_cnetIpv6OldMap[$key]}" ]; then
            CnetRepeatSendIpv6Nd "${key}"
            continue
        fi
        CnetDoNd "${key}"
    done
    return
}

function CnetRefreshIpv6()
{
    local key=""
    
    for key in ${!g_cnetIpv6OldMap[@]}; do
        if [ "${g_cnetIpv6Map[$key]}" != "${g_cnetIpv6OldMap[$key]}" ]; then
            CnetUnsetIpv6OldMap "${key}"
        fi   
    done
    for key in ${!g_cnetIpv6Map[@]}; do
        if [ "${g_cnetIpv6Map[$key]}" == "${g_cnetIpv6OldMap[$key]}" ]; then
            CnetUnsetIpv6Map "${key}"
            continue
        fi
        CnetSetIpv6Map "${key}"
        CnetUnsetIpv6Map "${key}"
    done
}

while true; do
    # ipv4 免费ARP通告
    CnetParseIpv4
    CnetSendIpv4Arp
    CnetRefreshIpv4
    
    # ipv6 免费ARP通告
    CnetParseIpv6
    CnetSendIpv6Nd
    CnetRefreshIpv6
    
    # 每隔2s，周期性例测
    sleep 2
done
