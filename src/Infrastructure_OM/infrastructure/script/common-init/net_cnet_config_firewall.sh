#!/bin/bash

function NET_CNET_SET_ICMP_FIREWALL()
{
    iptables -A INPUT -w -p icmp --icmp-type 13 -j DROP
    iptables -A INPUT -w -p icmp -j ACCEPT
    ip6tables -A INPUT -w -p icmpv6 --icmpv6-type 13 -j DROP
    ip6tables -A INPUT -w -p icmpv6 -j ACCEPT
    return 0
}

function NET_CNET_PARSE_PORTS()
{
    local item=$1
    local ports=`cat "$NET_CNET_FIREWALL_FILE" | grep "^${item}=" | awk -F'"' '{print $2}'`
    if [ "x$ports" = "x" ];then
        return 0
    fi
    # 判断端口有效性：数字开头、数字结尾、只包含[0-9,:]字符
    local ret=`echo $ports | sed 's/[0-9][0-9,:]*[0-9]//'`
    if [ "x$ret" != "x" ]; then
        echo "[NET_CNET_FIREWALL] param port[$item] contains illegal char."
        return 1
    fi

    # 判断每个数字合法性：port范围0-65535
    ret=`echo $ports | awk -F'[,:]' '{ for (i=1; i<=NF; i++) { if ($i<0 || $i>65535) print $i } }'`
    if [ "x$ret" != "x" ]; then
        echo "[NET_CNET_FIREWALL] param port[$ports] invalid, must in [0-65535]."
        return 1
    fi

    # iptables multiport --dports最多支持15个，这按照7个拆分(100:200，这种连续端口算两个)
    local ports_str=`echo $ports | awk -F, '{
        for (i = 1; i <= NF; i++) {
            if (i % 7 == 0) {
                printf $i" "
            } else if (i == NF) {
                printf $i
            } else {
                printf $i","
            }
        }
    }'`

    echo $ports_str
    return 0
}

function NET_CNET_SET_PROTOCOL_FIREWALL()
{
    local protocol=$1
    local version=$2
    local item=$3
    local cmd="iptables"
    local ports_str=""
    local ports=()
    
    if [ "$protocol" != "tcp" -a "$protocol" != "udp" ]; then
        echo "[NET_CNET_FIREWALL] Unknow protocol $protocol."
        return 1
    fi
    if [ "$version" != "ipv4" -a "$version" != "ipv6" ]; then
        echo "[NET_CNET_FIREWALL] Unknow version $version."
        return 1
    fi
    if [ "$version" = "ipv6" ]; then
        cmd="ip6tables"
    fi
    ports_str=$(NET_CNET_PARSE_PORTS $item)
    if [ $? -ne 0 ]; then
        echo $ports_str
        return 1
    fi
    if [ "x$ports_str" = "x" ];then
        return 0
    fi
    ports=(${ports_str//''/})
    for ((k=0; k<${#ports[@]}; k++))
    do
        $cmd -A INPUT -w -p $protocol -m multiport --dports ${ports[k]} -j ACCEPT
        echo "$cmd -A INPUT -w -p $protocol -m multiport --dports ${ports[k]} -j ACCEPT"
    done
    return 0
}

iptables -F INPUT
iptables -P INPUT DROP
iptables -A INPUT -w -m state --state RELATED,ESTABLISHED -j ACCEPT
ip6tables -F INPUT
ip6tables -P INPUT DROP
ip6tables -A INPUT -w -m state --state RELATED,ESTABLISHED -j ACCEPT

NET_CNET_FIREWALL_FILE=$1
if [ "x$NET_CNET_FIREWALL_FILE" = "x" ]; then
    NET_CNET_SET_ICMP_FIREWALL
    exit 0
fi

NET_CNET_SET_PROTOCOL_FIREWALL "tcp" "ipv4" "tcp_allow_ports"
NET_CNET_SET_PROTOCOL_FIREWALL "tcp" "ipv6" "tcp_allow_ports_ipv6"
NET_CNET_SET_PROTOCOL_FIREWALL "udp" "ipv4" "udp_allow_ports"
NET_CNET_SET_PROTOCOL_FIREWALL "udp" "ipv6" "udp_allow_ports_ipv6"
NET_CNET_SET_ICMP_FIREWALL
exit 0